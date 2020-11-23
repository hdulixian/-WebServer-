// Copyright (C) 2020 by Lixian. All rights reserved.
// Date: 2020-07-03
#include "Server.h"
#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include "Util.h"
#include "base/Logging.h"

Server::Server(EventLoop *loop, int threadNum, int port)
        : loop_(loop),
          threadNum_(threadNum),
          eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum_)),
          started_(false),
          port_(port),
          listenFd_(socket_bind_listen(port_)),
          listenChannel_(new Channel(loop_, listenFd_)) {
    handleSigpipe();
    if (setSocketNonBlocking(listenFd_) < 0) {
        perror("set socket non block failed");
        abort();
    }
}

void Server::start() {
    eventLoopThreadPool_->start();
    // ioLoop起来后添加对listenFd_的监听,否则在ioLoop起来之前所有已连接套接字上的IO事件都将在mainLoop上处理,
    // 违背了mainLoop只监听连接事件, ioLoop监听已连接套接字上的IO事件的原则。
    listenChannel_->setEvents(EPOLLIN | EPOLLET);
    listenChannel_->setReadHandler(bind(&Server::handNewConn, this));
    loop_->addToPoller(listenChannel_, 0);  /* epoll_ctl() */
    started_ = true;
}

void Server::handNewConn() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd = 0;

    while ((connfd = accept(listenFd_, (struct sockaddr *)&client_addr, &client_addr_len)) > 0) {
        EventLoop *loop = eventLoopThreadPool_->getNextLoop();
        LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port);
        std::cout << "new connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;

        if (connfd >= MAXFDS) { // 限制服务器的最大并发连接数
            close(connfd);
            continue;
        }
        if (setSocketNonBlocking(connfd) < 0) { // 设为非阻塞IO模式
            LOG << "Set non block failed!";
            return;
        }
        setSocketNodelay(connfd);

        shared_ptr<HttpData> req_info(new HttpData(loop, connfd));
        req_info->getChannel()->setHolder(req_info);
        loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));
    }
    
    listenChannel_->setEvents(EPOLLIN | EPOLLET); // 对应 getEventsRequest 将 events 置空
}