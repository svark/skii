//-*- mode:c++ -*-
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <limits.h>
#include <queue>
#include <algorithm>
#include <iostream>
#include <assert.h>


// compare function to sort locations to visit based on their altitude
struct CompareHeights
{
    CompareHeights(int w, int l, int **h)
        :width(w),length(l),heights(h)
    {
    }

    bool operator()(long p, long q)
    {
        return
            heights[p%length][p/length] >
            heights[q%length][q/length];
    }
    int width, length;
    int **heights;
};

void findSkiiPath( int width, int length,
                   int **heights)
{
    // we model this as a graph traversal problem where the given graph is a DAG 
    // with nodes at locations in the map and edges pointing n,e,w,s from tail nodes provided 
    // head of these edges is a node of greater altitude
    std::set<long> unvisitedNodes;
    for(int i =0 ; i<length*width;++i)
    {
        unvisitedNodes.insert(i);
    }
    std::vector<long> prev(width*length, -1), ascent(width*length, 0),
        len(width*length, 0);

    static bool debug = false;
    int numIter = 0;
    while(unvisitedNodes.size()) {

        ++numIter;
        std::set<long> levelSetAtMinHeight;
        int minHeight = INT_MAX;
        std::set<long>::iterator it = unvisitedNodes.begin();
        // among unvisitedNodes we would like to start traversing the
        // DAG from the bottom-most height
        for(;it!=unvisitedNodes.end();++it) {
            long idx = *it;
            int l = idx%length;
            int w = idx/width;
            if(heights[l][w] < minHeight)
                minHeight = heights[l][w];
        }

        it = unvisitedNodes.begin();
        for(;it!=unvisitedNodes.end();++it) {
            long idx = *it;
            int l = idx%length;
            int w = idx/width;
            if(heights[l][w]==minHeight)
                levelSetAtMinHeight.insert(idx);
        }

        std::priority_queue<long, std::vector<long> , CompareHeights >
            toVisit( CompareHeights(width, length, heights) );

        it = levelSetAtMinHeight.begin();
        std::set<long> unique;
        for(;it!=levelSetAtMinHeight.end();++it)
        {
            toVisit.push(*it);
            unique.insert(*it);
        }
        std::cout << '.';
        if(numIter % 80==0)
             std::cout << "\n";

        std::cout.flush();
        while(toVisit.size())
        {
            assert(toVisit.size()<  (unsigned int)width*length );
            long p = toVisit.top();
            unvisitedNodes.erase(p);
            int w = p/length;
            int l = p%length;
            if(debug){
                std::cout << "p:" << p << " lenp:" << len[p]
                          << ", ascent:" <<  ascent[p] 
                          << ", prev:" << prev[p] << std::endl;
            }
            assert(ascent[p] <= heights[l][w]);
            toVisit.pop();
            unique.erase(p);

            int delta[] = { -1,0, //west
                             1,0, //east
                             0,-1, //south
                             0,1 }; //north
            for(int k = 0; k < 4; ++k)
            {
                 int i = delta[2*k];
                 int j = delta[2*k+1];
                 if(
                    l + i >=0 && l + i < length // check extremes
                   && w + j >=0 && w + j < width
                   && heights[l + i][w + j] >  heights[l][w] // DAG has edges pointing up only
                   )
                {
                    int q = length*(w + j) + l + i;

                    if( prev[q]==-1 //  uninitialized
                        || (len[q] < len[p] + 1) //  prefer longer paths
                        || ( (len[q]  == len[p] + 1) && // otherwise
                                                      // prefer
                                                      // steeper paths
                             ascent[q] < ascent[p] + 
                                 (heights[l+i][w+j] - heights[l][w]) ))
                    {
                        prev[q] = p;
                        len[q] = len[p] + 1;
                        ascent[q] = ascent[p] + (heights[l+i][w+j] - heights[l][w]);
                        if(unique.find(q)==unique.end()) {
                            //  keep the queue of tovisit nodes without dups
                            toVisit.push(q);
                            unique.insert(q);
                        }

                    }
                }
            
            }
        }
    }
    int maxLen = 0;
    int maxIndex = 0;
    // look for the longest paths
    for(int i =0 ; i < width *length ; ++i)
    {
        if(len[i] > maxLen )
        {
            maxIndex = i;
            maxLen = len[i];
        }
    }
    // prefer steep paths among the longest
    for(int i = 0; i< width * length ; ++i) {
        if(len[maxIndex] == len[i]  && ascent[i] > ascent[maxIndex] )
        {
            maxIndex = i;
        }
    }
    // dump the longest path
    {
        std::cout << "\np: "  << maxIndex << std::endl;
        int p = maxIndex;
        for(int i = 0;i < len[maxIndex] + 1; ++i)
        {
            int w = p/length;
            int l = p%length;
            std::cout << '(' << w<< ","<< l <<  ") drop:"
                      << ascent[maxIndex] - ascent[p]  << " h:" << heights[l][w] << std::endl ;
            p = prev[p];
        }
    }
    std::cout << std::endl;
    std::cout <<  "answer " << len[maxIndex] + 1 << "," << ascent[maxIndex] << std::endl;
}

int main(int argc , char **argv)
{
    std::ifstream in("map.txt" );

    int width = 0;
    int length = 0;
    if(in.good())
    {
        in >> width >> length;
    }

    int ** heights = new int*[length];
    for(int i =0; i < length; ++i) {
        heights[i]  = new int[width];
        std::fill(heights[i],  heights[i] + width,  0);
    }

    long numRead=0;
    int  l = 0;
    for(;l < length && in.good() ; ++l)
    {
        for(int w = 0; w < width && in.good() ; ++w)
        {
            long height=0;
            in >> height;
            ++numRead;
            heights[l][w] = height;
        }
    }
    assert(numRead == width*length);
    findSkiiPath( width, length, heights);

    for(int i =0; i < length; ++i)
        delete [] heights[i];
    delete [] heights;
    return 0;
}
