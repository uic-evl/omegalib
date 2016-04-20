/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2015		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2015, Electronic Visualization Laboratory,
* University of Illinois at Chicago
* All rights reserved.
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer. Redistributions in binary
* form must reproduce the above copyright notice, this list of conditions and
* the following disclaimer in the documentation and/or other materials provided
* with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*-----------------------------------------------------------------------------
* What's in this file
*	Support for offscreen rendering to various targets
******************************************************************************/
#include "omega/WarpMesh.h"
#include "omega/DisplaySystem.h"
#include "omega/SystemManager.h"
#include "omega/glheaders.h"

#include <iostream>     // cout, endl
#include <fstream>      // fstream
#include <algorithm>    // copy
#include <iterator>     // ostream_operator
#include <vector>
#include <string>

#include <boost/tokenizer.hpp>

using namespace omega;

////////////////////////////////////////////////////////////////////////////////

static bool readWarpMeshCSV(const String& filename, std::vector<WarpMeshUtils::WarpMeshDataFields>& records)
{
    using namespace std;
    using namespace boost;

    ifstream in(filename.c_str());
    if (!in.is_open()) return false;

    typedef tokenizer< char_separator<char> > Tokenizer;
    char_separator<char> sep(" ,", "+");
    vector< string > vec;
    string line;

    static const char* sExpectedFields[] = {
        "GridX", "GridY", "PosX", "PosY", "U", "V", 0
    };


    // read header
    if(getline(in, line))
    {
        int i = 0;
        int j = 0;
        Tokenizer tok(line, sep);
        for(Tokenizer::const_iterator it = tok.begin(); it != tok.end() && sExpectedFields[i]; ++it, ++i)
        {
            if(*it != string(sExpectedFields[i]))
            {
                ofwarn("readWarpMeshCSV: unexpected CSV header field '%1%'!", %*it);
                ofwarn("readWarpMeshCSV: Required data fields are: '%1%' '%2%' '%3%' '%4%' '%5%' '%6%'",
                       %sExpectedFields[0] %sExpectedFields[1] %sExpectedFields[2]
                       %sExpectedFields[3] %sExpectedFields[4] %sExpectedFields[5] );

                return false;
            }
        }
    }

    while (getline(in,line))
    {
        stringstream ss;
        Tokenizer tok(line, sep);
        WarpMeshUtils::WarpMeshDataFields data;
        Tokenizer::const_iterator it = tok.begin();

        ss << *it;
        ss >> data.GridX;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << *it;
        ss >> data.GridY;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << *it;
        ss >> data.PosX;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << *it;
        ss >> data.PosY;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << *it;
        ss >> data.U;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << *it;
        ss >> data.V;
        ss.clear();

        records.push_back(data);
    }

    return true;
}
////////////////////////////////////////////////////////////////////////////////

Ref<WarpMeshGeometry> WarpMeshUtils::loadWarpMesh(const String& filename, bool hasFullPath)
{
    String path;
    if(!hasFullPath)
    {
        if(!DataManager::findFile(filename, path))
        {
            //ofmsg("LOOKUP: %1%%2%", %ogetdataprefix() %filename);
            if(!DataManager::findFile(ogetdataprefix() + filename, path))
            {
                // Try adding the
                ofwarn("WarpMeshUtils::loadWarpMesh: could not load %1%: file not found.", %filename);
                return NULL;
            }
        }
    }
    else
    {
        path = filename;
    }

    std::vector<WarpMeshUtils::WarpMeshDataFields> records;
    if(readWarpMeshCSV(path, records) == false)
    {
        ofwarn("WarpMeshUtils::loadWarpMesh: failed to parse data from file %1%: invalid file.", %filename);
        return NULL;
    }

    int startRow = -1;
    int startCol = -1;
    int maxRows = -1;
    int maxCols = -1;
    int missingRows = 0;
    int missingCols = 0;
    for(std::vector<WarpMeshUtils::WarpMeshDataFields>::const_iterator it = records.begin(); it != records.end(); ++it)
    {
        if(startRow < 0)
        {
            startRow = it->GridY;
        }

        if(startCol < 0)
        {
            startCol = it->GridX;
        }

        maxRows = std::max(maxRows, it->GridY);
        maxCols = std::max(maxCols, it->GridX);

//        ofmsg("readWarpMeshCSV: %1% %2% %3% %4% %5% %6%", %it->GridX %it->GridY %it->PosX %it->PosY %it->U %it->V);
    }

    ofmsg("readWarpMeshCSV: %1% x %2% -> %3% x %4% ", %startCol %startRow %maxCols %maxRows);

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////

WarpMesh::WarpMesh()
{
    // EMPTY!
}

WarpMesh::~WarpMesh()
{
    // EMPTY!
}

void WarpMesh::initialize() { myInitialized = true; }

void WarpMesh::prepare(Renderer* client, const DrawContext& context)
{
    // EMPTY!
}

void WarpMesh::render(Renderer* client, const DrawContext& context)
{
    // EMPTY!
}

void WarpMesh::dispose()
{
    // EMPTY!
}

///////////////////////////////////////////////////////////////////////////

