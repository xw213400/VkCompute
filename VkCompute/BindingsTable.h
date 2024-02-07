#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <sstream>


class BindingsTable
{
public:
    BindingsTable(std::string filename)
    {
        std::ifstream myFile(filename);

        if (!myFile.is_open())
        {
            throw std::runtime_error("Could not open csv file");
        }

        std::string line;

        while (std::getline(myFile, line))
        {
            _rows.push_back(split(line, ','));
        }

        myFile.close();
    }

    inline std::size_t size()
    {
        return _rows.size();
    }

    inline std::string getType(int i)
    {
        return _rows[i][1];
    }

    inline std::string getName(int i)
    {
        return _rows[i][2];
    }

    inline float getValue(int i)
    {
        return (float)atof(_rows[i][3].c_str());
    }

private:
    std::vector<std::vector<std::string>> _rows;

    std::vector<std::string> split(const std::string &s, char delim)
    {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;

        while (getline(ss, item, delim))
        {
            result.push_back(item);
        }

        return result;
    }
};
