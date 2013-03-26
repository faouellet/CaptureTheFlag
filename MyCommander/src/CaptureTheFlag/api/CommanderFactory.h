#ifndef COMMANDERFACTORY_H
#define COMMANDERFACTORY_H

#include <string>
#include <vector>
#include <map>
#include <functional>

#include "Commander.h"

class CommanderFactory
{
public:
    typedef std::function<Commander*()> Creator;

    void registerCommander(const std::string& name, Creator creator)
    {
        m_creators[name] = creator;
    }

    Commander* create(const std::string& name)
    {
        auto iter = m_creators.find(name);
        if (iter == m_creators.end())
        {
            return nullptr;
        }
        else
        {
            return iter->second();
        }
    }

    bool exists(const std::string& name)
    {
        return m_creators.find(name) != m_creators.end();
    }

    size_t getCommanderCount() const
    {
        return m_creators.size();
    }

    // Returns the names of all of the registered commanders.
    std::vector<std::string> getCommanderNames() const
    {
        std::vector<std::string> names;
        for (auto iter = m_creators.begin(); iter != m_creators.end(); ++iter)
        {
            names.push_back(iter->first);
        }
        return names;
    }

    static CommanderFactory& getInstance()
    {
        static CommanderFactory instance;
        return instance;
    }

private:
    std::map< std::string, Creator> m_creators;
};

class CommanderAutoRegister
{
public:
    CommanderAutoRegister(const std::string& name, CommanderFactory::Creator creator)
    {
        CommanderFactory::getInstance().registerCommander(name, creator);
    }
};

#define REGISTER_COMMANDER(COMMANDER) static CommanderAutoRegister s_autoRegister##COMMANDER(#COMMANDER, [] () { return new COMMANDER(); });


#endif // COMMANDERFACTORY_H
