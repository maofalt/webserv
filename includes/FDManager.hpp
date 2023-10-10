// FDManager.hpp

#ifndef FDMANAGER_HPP
#define FDMANAGER_HPP

#include "Server.hpp"

class FDManager {
public:
    static FDManager&           getInstance();
    static FDManager&           getInstance(Server& server);
    static void                 destroyInstance();

    const int&                  getEpollFd();
    const int&                  getSelfPipeReadFd();
    const int&                  getSelfPipeWriteFd();
    const std::vector<int>&     getSockListens();
    const std::set<int>&        getTrackFds();

    static void                  closeAllFds();

    void                        initialize(const Server& server);

private:
    FDManager(Server& server);
    ~FDManager();

    const int&                  _epoll_fd;
    const int&                  _selfPipeReadFd;
    const int&                  _selfPipeWriteFd;
    const std::vector<int>&     _sock_listens;
    const std::set<int>&        _trackFds;

    static FDManager*           _instance;
};

#endif // FDMANAGER_HPP
