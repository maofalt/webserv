class NginxConfig {
public:
    void validateAll();
    bool												isListeningTo(uint16_t port) const;
	bool												isNamed(const std::string &name) const;

private:
    struct MainConfig {
        std::map<std::string, std::vector<std::string> > data;
    } mainConfig;

    struct EventsConfig {
        std::map<std::string, std::vector<std::string> > data;
    } eventsConfig;

    struct HttpConfig {
        std::map<std::string, std::vector<std::string> > data;
        std::vector<ServerConfig> servers;
    } httpConfig;

    struct ServerConfig {
        std::map<std::string, std::vector<std::string> > data;
        std::vector<LocationConfig> locations;
    };

    struct LocationConfig {
        std::map<std::string, std::vector<std::string> > data;
    };
};
