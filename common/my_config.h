#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#include <map>
#include <string>

typedef std::map < std::string , std::string > config_section_t;
typedef std::map < std::string , config_section_t > config_data_t;

config_data_t config_load(const std::string & file_name);
void config_try_set_int(config_data_t & r, const std::string & section, const std::string & key, int & val);

#endif /* MY_CONFIG_H */

