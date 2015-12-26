//
//  CFileNode.h
//  LinuxProj
//
//  Created by MarK on 12/24/14.
//  Copyright (c) 2014 MarK. All rights reserved.
//

#ifndef LinuxProj_CFileNode_h
#define LinuxProj_CFileNode_h


#endif

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class CFileNode {

private:

    string nodeName;
    string nodePath;
    int nodeType; /*0 表示应用程序, 1 表示可用Vim打开的文本文件， 2 表示文件夹*/

public:

    int iLevel;
    int nodeSize;

    CFileNode()
    {
        nodeName = "";
        nodePath = "";
        nodeType = -1;
        iLevel = 9;

    }

    CFileNode(string name, string path, int type)
    {
        nodeName = name;
        nodePath = path;
        nodeType = type;
        iLevel = 9;
    }

    //open the current path, use command sudo nautilus /path
    void openCurrent()
    {
        int i = nodeName.find('.');
        string subStr = nodeName.substr(i+1, nodeName.length()-i-1);
        int iFlag = 0;

        vector<string> acceptTypes;
        acceptTypes.push_back("vim");
        acceptTypes.push_back("c");
        acceptTypes.push_back("h");
        acceptTypes.push_back("txt");
        acceptTypes.push_back("cpp");
        acceptTypes.push_back("md");
        string command = "";
        for(int i = 0; i< acceptTypes.size(); ++i)
        {
            if(subStr == acceptTypes[i])
            {
                string sPath = DeleteBlank(nodePath);
                string sName = DeleteBlank(nodeName);
                command = "gedit " + sPath + sName;
                iFlag = 1;
            }
        }

        if(iFlag == 0)
        {
            string sPath = DeleteBlank(nodePath);
            string sName = DeleteBlank(nodeName);
            command = "nautilus " + sPath + sName;
        }

        system(command.c_str());
    }

    // if the input string is contained in nodeName return ture, else return false
    bool containKeyWord(string str)
    {
        if(nodeName == str){
            iLevel = 1;
            return true;
        }

        if (nodeName.find(str) != nodeName.npos) {
            iLevel = 2;
            return true;
        }
        else
        {
            int i = nodeName.find('.');
            string subStr = nodeName.substr(i+1, nodeName.length()-i-1);

            vector<string> acceptTypes;
            acceptTypes.push_back("vim");
            acceptTypes.push_back("c");
            acceptTypes.push_back("h");
            acceptTypes.push_back("txt");
            acceptTypes.push_back("cpp");
            acceptTypes.push_back("md");

            for (int i = 0; i < acceptTypes.size(); ++i) {
                if (subStr == acceptTypes[i]) {

                    string strInput = nodePath + nodeName;



                    ifstream f(strInput.c_str(), ios::in);
                    if(!f){
                        return false;
                    }

                    char c;
                    string strContent = "";
                    while((c=f.get())!=EOF)
                    {
                        strContent += c;
                    }

                    f.close();


                    if(strContent.find(str) == strContent.npos)
                    {
                        return false;
                    }
                    else{
                        iLevel = 3;
                        return true;
                    }
                }
            }

            return false;
        }
    }

    void setNodeName(string name)
    {
        nodeName = name;
    }

    string getNodeName()
    {
        return nodeName;
    }

    void setNodePath(string path)
    {
        nodePath = path;
    }

    string getNodePath()
    {
        return nodePath;
    }

    void setNodeType(int type)
    {
        nodeType = type;
    }

    int getNodeType()
    {
        return nodeType;
    }

    string DeleteBlank(string text)
    {
        vector<string> newStr;

        for(int i = 0; i < text.length(); ++i)
        {
            if(text.substr(i,1) == " ")
            {
                newStr.push_back("\\ ");
            }
            else
            {
                newStr.push_back(text.substr(i,1));
            }
        }

        string newText = "";

        for(int i = 0; i < newStr.size(); ++i)
        {
            newText += newStr[i];
        }

        return newText;
    }
};
