#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <vector>
#include <map>
#include "Singleton.h"

namespace leo {

class Config {
public:
    friend class Singleton<Config>;

    /**
     * 定义一个存放配置信息的map
     * key 是string 存放一个标题section
     * value 是一个map 存放该标题下面的所有key-value键值对
     */ 
    typedef std::map<std::string, std::map<std::string, std::string> *> ConfigMap;

    ~Config();
    bool setPath(const std::string& path);
    std::string getString(const std::string& section, const std::string& key, const std::string& default_value = "");
    std::vector<std::string> getStringList(const std::string& section, const std::string& key);
    unsigned getNumber(const std::string& section, const std::string& key, unsigned default_value = 0);
    bool getBool(const std::string& section, const std::string& key, bool default_value = false);
    float getFloat(const std::string& section, const std::string& key, const float& default_value);
    
private:
    Config() {} 
    bool isSection(std::string line, std::string& section);
    unsigned parseNumber(const std::string& s);
    std::string trimLeft(const std::string& s);
    std::string trimRight(const std::string& s);
    std::string trim(const std::string& s);
    bool Load(const std::string& path);

    ConfigMap configs_;
};

}

#endif