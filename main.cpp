/*
*  Function: Divide each point into image block. The block has overlap and point in overlap area will belong to relative blocks at the sametimes.
*  Author: yangcong
*  Email: congyang96@163.com
*  History:
*    2021/08/28 - Create
*/
#include <vector>
#include <opencv2/opencv.hpp>

#define MAX(a, b) ((a)>(b)?(a):(b))
#define MIN(a, b) ((a)<(b)?(a):(b))
#define CLAMP(a, min, max) MIN(MAX((a), (min)), (max))

using namespace std;
using namespace cv;

vector<vector<int>> CreatePointList(int h, int w)
{
    vector<int> xs, ys;
    for (int i = 10; i < h - 10; i += 27)
    {
        for (int j = 10; j < w - 10; j += 27)
        {
            xs.push_back(j);
            ys.push_back(i);
        }
    }
    return { xs, ys };
}

Mat ShowCluster(vector<vector<int>> cluster, vector<vector<int>> pts, int* block_sz, int* pad, int h, int w)
{
    Mat img = Mat::zeros(h, w, CV_8UC3);
    for (int i = 0; i < h; i += block_sz[1])
    {
        line(img, Point(0, i), Point(w - 1, i), Scalar(255, 255, 255));
    }
    for (int j = 0; j < w; j += block_sz[0])
    {
        line(img, Point(j, 0), Point(j, h - 1), Scalar(255, 255, 255));
    }

    int bk_num = cluster.size();
    for (int b = 0; b < bk_num; ++b)
    {
        vector<int> pt_list = cluster[b];
        for (int id : pt_list)
        {
            int x = pts[0][id];
            int y = pts[1][id];
            circle(img, Point(x, y), 2, Scalar(125, 255, 120));
            putText(img, String(to_string(b)), Point(x + b, y), 1, 0.6, Scalar(255 * b / bk_num, 255 - 255 * b / bk_num, 125));
        }
    }
    return img;
}

vector<vector<int>> DividePointIntoBlock(const vector<vector<int>>& pts, int* block_sz, int* pad, int h, int w)
{
    int bk_col_max = w / block_sz[0];
    int bk_row_max = h / block_sz[1];
    int bk_num = bk_col_max * bk_row_max;
    vector<vector<int>> output(bk_num, vector<int>());

    // for each pt, divide it into blocks (may be multiply blocks)
    for (int ip = 0; ip < pts[0].size(); ++ip)
    {
        if (ip == 27)
             cout << ip;
        int xp = pts[0][ip];
        int yp = pts[1][ip];
        int bk_x = xp / block_sz[0];
        int bk_y = yp / block_sz[1];
        if (bk_x >= bk_col_max || bk_y >= bk_row_max)
        {
            continue;
        }
        // (xp, yp) must belong to (bk_x, bk_y)
        output[bk_y * bk_col_max + bk_x].push_back(ip);

        // deal with overlap
        int x_in_bk = xp - bk_x * block_sz[0];  // x relative to block left-top
        int y_in_bk = yp - bk_y * block_sz[1];  // y relative to block left-top
        int x_offset = 0;
        int y_offset = 0;
        if (x_in_bk < pad[0])
            x_offset = -1;
        else if (x_in_bk > (block_sz[0] - pad[0]))
            x_offset = 1;
        int new_x = bk_x + x_offset;

        if (y_in_bk < pad[1])
            y_offset = -1;
        else if (y_in_bk > (block_sz[1] - pad[1]))
            y_offset = 1;
        int new_y = bk_y + y_offset;

        int flag = 0;
        // x direction
        if (new_x >= 0 && new_x < bk_col_max && x_offset != 0)
        {
            output[bk_y * bk_col_max + new_x].push_back(ip);
            ++flag;
        }
        // y direction
        if (new_y >= 0 && new_y < bk_row_max && y_offset != 0)
        {
            output[new_y * bk_col_max + bk_x].push_back(ip);
            ++flag;
        }
        // corner direction, last two if both meet
        if (flag == 2)
        {
            output[new_y * bk_col_max + new_x].push_back(ip);
        }
    }

    return output;
}

int main()
{
    int h = 1500;
    int w = 2000;
    int block_sz[2] = { 450, 200 };  // x, y direction
    int pad[2] = { 50, 50 };  // x, y direction

    vector<vector<int>> pts = CreatePointList(h, w);
    vector<vector<int>> cluster = DividePointIntoBlock(pts, block_sz, pad, h, w);
    Mat img = ShowCluster(cluster, pts, block_sz, pad, h, w);
    imwrite("res.tif", img);
}