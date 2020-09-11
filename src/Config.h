#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <vector>
#include <map>
#include "Singleton.h"

namespace leo {

//设计成单例模式
class Config {
public:
    friend class Singleton<Config>;

    //定义一个存放配置信息的map
    //key 是string 存放一个标题section
    //value 是一个map 存放该标题下面的所有key-value键值对
    typedef std::map<std::string, std::map<std::string, std::string> *> STR_MAP;

    ~Config();

    //获取字符串类型配置信息
    std::string getString(const std::string& section, const std::string& key, const std::string& default_value = "");

    //字符串集合配置信息
    std::vector<std::string> getStringList(const std::string& section, const std::string& key);

    //获取整型类型配置信息
    unsigned getNumber(const std::string& section, const std::string& key, unsigned default_value = 0);

    //获取布尔类型配置信息
    bool getBool(const std::string& section, const std::string& key, bool default_value = false);

    //获取浮点类型配置信息
    float getFloat(const std::string& section, const std::string& key, const float& default_value);

    //设置配置文件所在路径
    bool setPath(const std::string& path);
    
private:
    Config() { } //构造私有

    
    //字符串配置文件解析基础方法
    bool isSection(std::string line, std::string& section);
    unsigned parseNumber(const std::string& s);
    std::string trimLeft(const std::string& s);
    std::string trimRight(const std::string& s);
    std::string trim(const std::string& s);
    bool Load(const std::string& path);

    STR_MAP configs_;
};

}

#endif