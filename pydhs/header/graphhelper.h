//
//  graphhelper.h
//  MyGraph
//
//  Created by tonny.achilles on 5/29/14.
//  Copyright (c) 2014 tonny.achilles. All rights reserved.
//

#ifndef GRAPHHELPER_H
#define GRAPHHELPER_H

#include <boost/algorithm/string.hpp>
#include <fstream>
#include "PREDEFINE.h"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <unordered_map>
using namespace std;
using namespace boost;
void graph_matsim2csv(string _input, string _output)
{
    ofstream fout(_output);
    // ------------------------------------ anaylze network.xml ---------------------------------------
    cout << "read network.xml" << endl;
    property_tree::ptree *pt_network = new property_tree::ptree();
    property_tree::xml_parser::read_xml(_input, *pt_network);
    auto links = pt_network->get_child("network.links");
    
    delete pt_network;
    
    int e_idx = 0;
    for (const auto &link : links)
    {
        if (link.first == "link")
        {
            auto fv_id = link.second.get<string>("<xmlattr>.from");
            auto tv_id = link.second.get<string>("<xmlattr>.to");
            auto e_id = link.second.get<string>("<xmlattr>.id");
            auto length = link.second.get<float>("<xmlattr>.length");
            auto freespeed = link.second.get<float>("<xmlattr>.freespeed");
            fout << e_id << ',' << fv_id <<',' << tv_id << ',' << length << ',' << freespeed << endl;
            e_idx++;
        }
    }
    
    fout.close();
}



unordered_map<string, unordered_map<int, float>> get_profile(vector<int> & time_second_keys)
{
    string profile_path = PROFILE_TOKYO_MATSIM_CSV_PATH;
    ifstream ins;
    //    if (ins.good()) cout << "file ok"<<endl;
    ins.open(profile_path);
    string line = "";
    int cnt = 1;
    unordered_map<string, unordered_map<int, float>> profile; // profile[long linkid][time_of_day]
    while (ins) {
        //        std::getline(ins,line);
        ins >> line;
        std::vector<std::string> strs;
        boost::split(strs, line, boost::is_any_of(","));
        
        if (cnt == 1) {
            for (auto str : strs) {
                if (str != "")
                    time_second_keys.push_back(std::stoi(str));
            }
        } else {
            for (int i = 1; i < strs.size(); ++i) {
                string e_id = strs[0];
                int time_second_key = time_second_keys[i - 1];
                float value = std::stof(strs[i]);
                profile[e_id][time_second_key] = value;
            }
        }
        cnt++;
    }
    ins.close();
    return profile;
}

#endif
