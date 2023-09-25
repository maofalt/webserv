#include "FDManager.hpp"


FDManager* FDManager::_instance = NULL;

FDManager::FDManager(Server& server) 
    : _epoll_fd(server.getEpollFd()),
    _selfPipeReadFd(server.getSelfPipeReadFd()),
    _selfPipeWriteFd(server.getSelfPipeWriteFd()),
    _sock_listens(server.getSockListens()),
    _trackFds(server.getTrackFds())
    {}

FDManager::~FDManager() {
}

FDManager& FDManager::getInstance() {
    if (_instance == NULL) {
        throw std::runtime_error("FDManager not initialized");
    }
    return *_instance;
}

void FDManager::destroyInstance() {
    if (_instance != NULL) {
        delete _instance;
        _instance = NULL;
    }
}

FDManager& FDManager::getInstance(Server& server) {
    if (_instance == NULL) {
        _instance = new FDManager(server);
    }
    return *_instance;
}

const int& FDManager::getEpollFd() {
    return _epoll_fd;
}

const int& FDManager::getSelfPipeReadFd() {
    return _selfPipeReadFd;
}

const int& FDManager::getSelfPipeWriteFd() {
    return _selfPipeWriteFd;
}

const std::vector<int>& FDManager::getSockListens() {
    return _sock_listens;
}

const std::set<int>& FDManager::getTrackFds() {
    return _trackFds;
}

void FDManager::closeAllFds() {
    FDManager& instance = FDManager::getInstance();

    for (std::vector<int>::const_iterator it = instance._sock_listens.begin();
        it != instance._sock_listens.end(); ++it) {
        close(*it);
    }
    close(instance._selfPipeReadFd);
    close(instance._selfPipeWriteFd);
    close(instance._epoll_fd);
    for (std::set<int>::const_iterator it = instance._trackFds.begin();
        it != instance._trackFds.end(); ++it) {
        close(*it);
    }
}
