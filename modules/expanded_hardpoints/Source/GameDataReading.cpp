#include "PCH.hpp"

#include "ExpandedHardpoints.hpp"


void ExpandedHardpoints::LoadHardpointData() { 

	INI_Reader ini;

	
	char szCurDir[MAX_PATH];
    GetCurrentDirectoryA(sizeof(szCurDir), szCurDir);
    std::string currDir = szCurDir;
    std::string scFreelancerIniFile = currDir + R"(\freelancer.ini)";
    std::string gameDir = currDir.substr(0, currDir.length() - 4) + "\\DATA\\";

    if (!ini.open(scFreelancerIniFile.c_str(), false))
    {
        return;
    }

    std::vector<std::string> equipFiles;
    std::vector<std::string> shipFiles;

    while (ini.read_header())
    {
        if (!ini.is_header("Data"))
        {
            continue;
        }
        while (ini.read_value())
        {
            if (ini.is_value("equipment"))
            {
                equipFiles.push_back(ini.get_value_string());
            }
            else if (ini.is_value("ships"))
            {
                shipFiles.push_back(ini.get_value_string());
            }
        }
    }

    ini.close();

    int maxArmorValue = 0;
    for (auto& shipFile : shipFiles)
    {
        shipFile = gameDir + shipFile;
        if (!ini.open(shipFile.c_str(), false))
        {
            continue;
        }

        Id currNickname;
        while (ini.read_header())
        {
            if (!ini.is_header("Ship"))
            {
                continue;
            }
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    currNickname = CreateID(ini.get_value_string());
                }
                else if (ini.is_value("hp_subtype"))
                {
                    for (int i = 1; !ini.is_value_empty(i); ++i)
                    {
                        shipIdToSubclassesMap[currNickname][CreateID(ini.get_value_string(0))].push_back(ini.get_value_string(i));
                    }
                }
            }

        }
        ini.close();
    }

    for (auto& equipFile : equipFiles)
    {
        equipFile = gameDir + equipFile;

        if (!ini.open(equipFile.c_str(), false))
        {
            continue;
        }

        Id currNickname;
        while (ini.read_header())
        {
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    currNickname = CreateID(ini.get_value_string());
                }
                else if (ini.is_value("hp_subtype"))
                {
                    for (int i = 0; !ini.is_value_empty(i); ++i)
                    {
                        equipIdToSubclassesMap[currNickname].insert(CreateID(ini.get_value_string(i)));
                    }
                }
            }
        }
    }

	Fluf::Info("Story Factions: Applied patches successfully");

}
