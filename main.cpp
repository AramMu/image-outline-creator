#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
//using namespace cv;

bool isBoundary(const cv::Mat& input, cv::Point p) {
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0) {
                continue;
            }
            int y = p.y+i;
            int x = p.x+j;
            if (x < 0 || y < 0 || x >= input.cols || y >= input.rows) {
                continue;
            }
            if (!input.at<uchar>(y,x)) {
                return true;
            }
        }
    }
    return false;
}

template <typename T>
T sqr(T x) {
    return x*x;
}

double distanc(cv::Point p1, cv::Point p2) {
    return sqrt(sqr(p1.x-p2.x) + sqr(p1.y-p2.y));
}

cv::Mat process(cv::Mat input, double dist) {
    vector<cv::Point> outer;
    vector<cv::Point> inner;
    for (int i = 0; i < input.rows; ++i) {
        for (int j = 0; j < input.cols; ++j) {
            if (input.at<uchar>(i,j)) {
                if (isBoundary(input, {j,i})) {
                    outer.push_back({j,i});
                } else {
                    inner.push_back({j,i});
                    //input.at<uchar>(i,j) = 0;
                }
            }
        }
    }
    sort(outer.begin(), outer.end(), [] (cv::Point p1, cv::Point p2) {
        return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
    });
    ///X position, low and high values are inclusive
    vector<int> xPosLo(input.cols, -1);
    vector<int> xPosHi(input.cols, -1);

    for (int i = 0; i < outer.size(); ++i) {
        if (xPosLo[outer[i].x] < 0) {
            xPosLo[outer[i].x] = i;
        }
        xPosHi[outer[i].x] = i;
    }

    for (int k = 0; k < inner.size(); ++k) {
        int xCen = inner[k].x;
        int xLo = -1, xHi = -1;
        for (int i = max(int(floor(xCen-dist)), 0); i <= xCen; ++i) {
            if (xPosLo[i] >= 0) {
                xLo = i;
                break;
            }
        }
        for (int i = min(int(ceil(xCen+dist)), input.cols-1); i >= xCen; --i) {
            if (xPosHi[i] >= 0) {
                xHi = i;
                break;
            }
        }
        if (xLo < 0 && xHi < 0) {
            input.at<uchar>(inner[k]) = 0;
            continue;
        }
        if (xLo < 0) {
            xLo = xCen;
        }
        if (xHi < 0) {
            xHi = xCen;
        }
        bool keep = false;
        for (int i = xLo; i <= xHi; ++i) {
            int indLo = xPosLo[i];
            int indHi = xPosHi[i];
            for (int j = indLo; j <= indHi; ++j) {
                if (distanc(inner[k], outer[j]) <= dist) {
                    keep = true;
                }
            }
        }
        if (!keep) {
            input.at<uchar>(inner[k]) = 0;
        }
    }

    cout << outer.size() << " " << inner.size() << endl;
    return input;
}

int main()
{
    cv::namedWindow("aaa", cv::WINDOW_NORMAL);
    cv::Mat img = cv::imread("Files/1.png", cv::IMREAD_GRAYSCALE);
    cv::resize(img, img, cv::Size(img.cols, img.rows*2.0), 0, 0, cv::INTER_NEAREST);
    cv::Mat res = process(~img, 7.5);
    cv::imwrite("Files/res.png", res);
    cv::imshow("aaa", res);
    cv::waitKey(0);
    return 0;
}
