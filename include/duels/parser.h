#ifndef DUELS_ARGPARSER_H
#define DUELS_ARGPARSER_H
#include <string>
#include <sstream>
#include <ostream>
#include <algorithm>

namespace duels
{
// register player names and base port
// [-n1 name1] [-n2 name2] [-p port] [--nodisplay]
// port +0 -> p1 in
// port +1 -> p2 in
// port +2 -> displays out
// port +3 -> shake display 1 in
// port +4 -> shake display 2 in

inline void run(const std::string &cmd)
{
    [[maybe_unused]] auto ret(system(cmd.c_str()));
}

inline void run(const std::stringstream &cmd)
{
    run(cmd.str());
}

#ifndef DUELS_SERVER
inline void killAll(std::string proc)
{
  run("killall " + proc + " -9 -q");
}
#endif

class ArgParser
{
public:
    ArgParser(int argc, char** argv, std::string ip = "*") : ip(ip)
    {
        // read overloaded ip / port
        for(int arg = 0; arg < argc; ++arg)
        {
            const std::string key(argv[arg]);
            if(key == "-p")
                updateFrom(argv[++arg]);

            else if(key == "-ip")
                ip = argv[++arg];

            else if(key == "--nodisplay")
                display = false;

            else if(key == "-n1")
                name1 = argv[++arg];

            else if(key == "-n2")
                name2 = argv[++arg];
        }
    }
    inline bool isRemote() const
    {
        return ip != "localhost";
    }
    inline bool localServer() const
    {
        return port_arg == 3000;
    }
    inline bool updateFrom(std::string arg)
    {
        port_arg = std::stoi(arg);
        if(port_arg == 0)
        {
            port_arg = 3000;
            ip = "localhost";
            return false;
        }
        return true;
    }

    inline int port() const
    {
#ifdef DUELS_SERVER
        return port_arg - (port_arg % 5);
#else
        return port_arg;
#endif
    }
    inline std::string serverIP() const
    {
        return ip;
    }
    inline bool isPlayer1() const
    {
        return port_arg % 5 == 0;
    }
    inline int displayPort() const
    {
        return display ? port() + 3 : 0;
    }
#ifdef DUELS_SERVER
    inline std::string clientURL(int player = 1) const
    {
        return url(player-1);
    }
    inline std::string shakeURL(int player = 1) const
    {
        return url(player+2);
    }
    inline std::string displayURL() const
    {
        return url(2);
    }
    inline std::pair<std::string, bool> extractPlayer1(int ai_fallback) const
    {
        return playerInfo(name1, ai_fallback);
    }
    inline std::pair<std::string, bool> extractPlayer2(int ai_fallback) const
    {
        return playerInfo(name2, ai_fallback);
    }
#else
    inline void setupPlayer2()
    {
        port_arg ++;
    }
    inline std::string serverURL() const
    {
        return url();
    }
#endif
private:
    int port_arg = 3000;    
    std::string ip, name1, name2;
    bool display = true;
    inline std::string url(int offset = 0) const
    {
        return "tcp://" + ip + ":" + std::to_string(port()+offset);
    }
#ifdef DUELS_SERVER
    inline std::pair<std::string, bool> playerInfo(const std::string &name, int ai_fallback) const
    {
        std::pair<std::string, bool> ret{name, false};
        if(name.empty())
        {
            // AI with fallback level
            ret.first = std::to_string(ai_fallback);
            ret.second = true;
        }
        else if(std::all_of(name.begin(), name.end(), ::isdigit))
        {
            // name is a requested AI level
            ret.second = true;
        }
        return ret;
    }
#endif
};


}

#endif
