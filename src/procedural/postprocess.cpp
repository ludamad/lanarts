/*
 * postprocess.cpp
 *
 *  Created on: Apr 15, 2012
 *      Author: 100397561
 */

#include "GeneratedLevel.h"
#include <queue>

//void connected_region(GeneratedLevel& level, const Pos& source, RegionType& region, const Color target){
//    Color src_color = color_of(source, region);
//    std::queue<Pos> analyze_queue;
//    analyze_queue.push(source);
//
//    while (!analyze_queue.empty())
//    {
//        if (color_of(analyze_queue.front()) != src_color)
//        {
//                analyze_queue.pop();
//                continue;
//        }
//        Point leftmost_pt = analyze_queue.front();
//            leftmost_pt.col -= 1;
//        analyze_queue.pop();
//        Point rightmost_pt = leftmost_pt;
//            rightmost_pt.col += 2;
//        while (color_of(leftmost_pt, region) == src_color)
//                --leftmost_pt.col;
//
//        while (color_of(rightmost_pt, region) == src_color)
//                ++rightmost_pt.col;
//
//        bool check_above = true;
//        bool check_below = true;
//            Point pt = leftmost_pt;
//            ++pt.col;
//        for (; pt.col < rightmost_pt.col; ++pt.col)
//        {
//                set_color(pt, region, target);
//
//                Point pt_above = pt;
//                    --pt_above.row;
//                if (check_above)
//                {
//                        if (color_of(pt_above, region) == src_color)
//                        {
//                                analyze_queue.push(pt_above);
//                                check_above = false;
//                        }
//                }
//                else // !check_above
//                {
//                        check_above = (color_of(pt_above, region) != src_color);
//                }
//
//                Point pt_below = pt;
//                    ++pt_below.row;
//                if (check_below)
//                {
//                        if (color_of(pt_below, region) == src_color)
//                        {
//                                analyze_queue.push(pt_below);
//                                check_below = false;
//                        }
//                }
//                else // !check_below
//                {
//                        check_below = (color_of(pt_below, region) != src_color);
//                }
//        } // for
//    } // while queue not empty
//}
