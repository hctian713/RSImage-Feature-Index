//
//遥感原理与方法课程设计-基于特征指数的遥感专题信息提取程序
//
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>   
#include <opencv2/highgui/highgui.hpp> 
using namespace cv;
using namespace std;

///////////////////////////阈值分割算法////////////////////////////////////
//迭代法
Mat Literation(const Mat &srcImg) {
    //创建整型灰度图
    Mat grayImg, dst;
    srcImg.convertTo(grayImg, CV_8UC1);
    dst = Mat(grayImg.size(), CV_8UC1);
    int height = grayImg.rows;//行数
    int width = grayImg.cols;//列数
    double minVal, maxVal;
    minMaxLoc(srcImg, &minVal, &maxVal);
    int initTK = (minVal + maxVal) / 2;
    int tempTK = initTK;
    int TK;
    int foreGrd,backGrd;
    int foreSum,backSum;
    while (true) {
        foreGrd = 0; backGrd = 0;
        foreSum = 0; backSum = 0;
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++) {
                int gray = grayImg.at<uchar>(i, j);
                if (gray >= tempTK) {
                    foreSum = foreSum + gray;
                    foreGrd++;
                }
                else {
                    backSum = backSum + gray;
                    backGrd++;
                }
            }
        TK = (foreSum / foreGrd + backSum / backGrd) / 2;
        if (TK != tempTK)
            tempTK = TK;
        else
            break;
    }
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++) {
            int gray = grayImg.at<uchar>(i, j);
            dst.at<uchar>(i, j) = gray >= tempTK ? 255 : 0;
        } 
    return dst;
}
//最大类间误差法
Mat Otus(const Mat& srcImg)
{
    //创建灰度图
    Mat grayImg, dst;
    srcImg.convertTo(grayImg, CV_8UC1);
    dst = Mat(grayImg.size(), CV_8UC1);
    int height = grayImg.rows;//行数
    int width = grayImg.cols;//列数
    //判断分析法（Ostu）获取阙值
    int nThreshold = 0;
    int hist[256]; memset(hist, 0, sizeof(hist));//定义灰度统计数组
    int lS, lS1, lS2;//总像素数，类1,2像素数
    double lP, lP1;//总质量矩，类1质量矩
    double meanvalue1, meanvalue2;//类1,2灰度均值
    int IterationTimes;//迭代次数
    double Dis1, Dis2, CinDis, CoutDis, max;//方差，类1，类2，组内，组间
    int graymax = 255, graymin = 0;//灰度最大最小值
    int i, j, k;
    //统计出各个灰度值的像素数
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++) {
            int gray = grayImg.at<uchar>(i, j);
            hist[gray]++;
        }
    lP = 0.0; lS = 0;
    if (graymin == 0) graymin++;
    //计算总的质量矩lP和像素总数lS
    for (k = graymin; k <= graymax; k++) {
        lP += double(k) * double(hist[k]);
        lS += hist[k];
    }
    max = 0.0;
    lS1 = lS2 = 0;
    lP1 = 0.0;
    Dis1 = Dis2 = CinDis = CoutDis = 0.0;
    double ratio = 0.0;
    //求阙值
    for (k = graymin; k < graymax; k++) {
        lS1 += hist[k];
        if (!lS1) { continue; }
        lS2 = lS - lS1;
        if (lS2 == 0) { break; }
        lP1 += double(k) * double(hist[k]);
        meanvalue1 = lP1 / lS1;
        for (int n = graymin; n <= k; n++)
            Dis1 += double((n - meanvalue1) * (n - meanvalue1) * hist[n]);
        meanvalue2 = (lP - lP1) / lS2;
        for (int m = k + 1; m < graymax; m++)
            Dis2 += double((m - meanvalue2) * (m - meanvalue2) * hist[m]);
        CinDis = Dis1 + Dis2;
        CoutDis = double(lS1) * double(lS2) * (meanvalue1 - meanvalue2) * (meanvalue1 - meanvalue2);
        if (CinDis != 0) ratio = CoutDis / CinDis;
        if (ratio > max) {
            max = ratio; nThreshold = k;
        }
    }
    //二值化图像
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++)
            dst.at<uchar>(i, j) = grayImg.at<uchar>(i, j) > nThreshold ? 255 : 0;
    return dst;
}

///////////////////////////////植被指数/////////////////////////////////

//比值植被指数 RVI=NIR/R
Mat getRVI(Mat* bands){
    Mat RVI = bands[4] / bands[3];
    normalize(RVI, RVI, 0.0, 255.0, NORM_MINMAX);
    RVI.convertTo(RVI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/RVI.jpg", RVI);
    RVI = Literation(RVI);
    imwrite("特征指数的专题信息图/阈值分割结果/RVI.jpg", RVI);
    return RVI;
}
//归一化植被指数 NDVI=(NIR-R)/(NIR+R)
Mat getNDVI(Mat* bands) {
    Mat NDVI = (bands[4] - bands[3]) / (bands[4] + bands[3]);
    normalize(NDVI, NDVI, 0, 255, NORM_MINMAX);
    NDVI.convertTo(NDVI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/NDVI.jpg", NDVI);
    NDVI = Literation(NDVI);
    imwrite("特征指数的专题信息图/阈值分割结果/NDVI.jpg", NDVI);
    return NDVI;
}
//土壤调节植被指数 SAVI=(NIR-R)/(NIR+R+L)*(1+L)
Mat getSAVI(Mat* bands) {
    Mat SAVI = (bands[4] - bands[3]) / (bands[4] + bands[3] + 0.5) * (1 + 0.5);
    normalize(SAVI, SAVI, 0, 255, NORM_MINMAX);
    SAVI.convertTo(SAVI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/SAVI.jpg", SAVI);
    SAVI = Literation(SAVI);
    imwrite("特征指数的专题信息图/阈值分割结果/SAVI.jpg", SAVI);
    return SAVI;
}
//三波段梯度差植被指数 TGDVI
Mat getTGDVI(Mat* bands,float* lambda) {
    Mat TGDVI = (bands[4] - bands[3]) / (lambda[4] - lambda[3]) 
        - (bands[3] - bands[2]) / (lambda[3] - lambda[2]);
    normalize(TGDVI, TGDVI, 0, 255, NORM_MINMAX);
    TGDVI.convertTo(TGDVI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/TGDVI.jpg", TGDVI);
    TGDVI = Literation(TGDVI);
    imwrite("特征指数的专题信息图/阈值分割结果/TGDVI.jpg", TGDVI);
    return TGDVI;
}   

///////////////////////////////水体指数/////////////////////////////////

//NDWI=(G-NIR)/(G+NIR)
Mat getNDWI(Mat* bands) {
    Mat NDWI = (bands[2] - bands[4]) / (bands[2] + bands[4]);
    normalize(NDWI, NDWI, 0, 255, NORM_MINMAX);
    NDWI.convertTo(NDWI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/NDWI.jpg", NDWI);
    NDWI = Literation(NDWI);
    imwrite("特征指数的专题信息图/阈值分割结果/NDWI.jpg", NDWI);
    return NDWI;
}
//MNDWI=(G-SWIR)/(G+SWIR)
Mat getMNDWI(Mat* bands) {
    Mat MNDWI = (bands[2] - bands[7]) / (bands[2] + bands[7]);
    normalize(MNDWI, MNDWI, 0, 255, NORM_MINMAX);
    MNDWI.convertTo(MNDWI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/MNDWI.jpg", MNDWI);
    MNDWI = Literation(MNDWI);
    imwrite("特征指数的专题信息图/阈值分割结果/MNDWI.jpg", MNDWI);
    return MNDWI;
}
//EWI=(G-(NIR+MIR))/(G+(NIR+MIR))
Mat getEWI(Mat* bands) {
    Mat EWI = (bands[2] - (bands[4] + bands[5])) / (bands[2] + (bands[4] + bands[5]));
    normalize(EWI, EWI, 0, 255, NORM_MINMAX);
    EWI.convertTo(EWI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/EWI.jpg", EWI);
    EWI = Literation(EWI);
    imwrite("特征指数的专题信息图/阈值分割结果/EWI.jpg", EWI);
    return EWI;
}

///////////////////////////////建筑用地指数/////////////////////////////////

//DBI 差值建筑覆盖指数 
Mat getDBI(Mat* bands) {

    Mat DBI = -(bands[7] - bands[4]);
    normalize(DBI, DBI, 0, 255, NORM_MINMAX);
    DBI.convertTo(DBI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/DBI.jpg", DBI);
    DBI = Literation(DBI);
    imwrite("特征指数的专题信息图/阈值分割结果/DBI.jpg", DBI);
    return DBI;
}
//NDBI 归一化差值建筑用地指数
Mat getNDBI(Mat* bands) {
    Mat NDBI = (bands[5] - bands[4]) / (bands[5] + bands[4]);
    normalize(NDBI, NDBI, 0, 255, NORM_MINMAX);
    NDBI.convertTo(NDBI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/NDBI.jpg", NDBI);
    NDBI = Literation(NDBI);
    imwrite("特征指数的专题信息图/阈值分割结果/NDBI.jpg", NDBI);
    return NDBI;
}
//NDBBI 归一化差值裸地与建筑用地指数
Mat getNDBBI(Mat* bands) {
    Mat NDBBI = (1.5 * bands[5] - (bands[4] + bands[2] / 2))
        / (1.5 * bands[5] + (bands[4] + bands[2] / 2));
    normalize(NDBBI, NDBBI, 0, 255, NORM_MINMAX);
    NDBBI.convertTo(NDBBI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/NDBBI.jpg", NDBBI);
    NDBBI = Literation(NDBBI);
    imwrite("特征指数的专题信息图/阈值分割结果/NDBBI.jpg", NDBBI);
    return NDBBI;
}
//BSI 裸土指数
Mat getBSI(Mat* bands) {
    Mat BSI = ((bands[5] + bands[3]) - (bands[4] + bands[1])) 
        / ((bands[5] + bands[3]) + (bands[4] + bands[1]));
    normalize(BSI, BSI, 0, 255, NORM_MINMAX);
    BSI.convertTo(BSI, CV_8UC1);
    imwrite("特征指数的专题信息图/直接提取结果/BSI.jpg", BSI);
    BSI = Literation(BSI);
    imwrite("特征指数的专题信息图/阈值分割结果/BSI.jpg", BSI);
    return BSI;
}

/////////////////////////////////TM影像参数///////////////////////////////////
//TM - 1为0.45～0.52微米，蓝光波段；
//TM - 2为0.52～0.60微米，绿光波段；
//TM - 3为0.63～0.69微米，红光波段，以上3段为可见光波段；
//TM - 4为0.76～0.90微米，为近红外波段；
//TM - 5为1.55～1.75微米，近红外波段；
//TM - 6为10.40～12.50微米，为热红外波段；
//TM - 7为2.08～2.35微米，近-中红外波段。
int main()
{
    //读入多波段，统一索引下标和波段序号
    int n = 7;//波段数7
    Mat* bands = new Mat[n+1];//索引0设置为空
    Mat temp;
    float lambda[] = { 0,0.07,0.08,0.06, 0.14,0.2,2.1,0.27 };//波段长度
    for (int i = 1; i <= n; i++) {//读取TM多光谱影像1-7波段波段影像
        string filepath = string("第二部分 专题指数\\tm") + to_string(i) + string(".tif");
        temp = imread(filepath,CV_8UC1);//读取无符号单通道影像
        temp.convertTo(bands[i], CV_32FC1);//转换为浮点型，便于后续计算指数
    }
 

    //比值植被指数 RVI=NIR/R
    getRVI(bands);
    //归一化植被指数 NDVI=(NIR-R)/(NIR+R)
    getNDVI(bands);
    //土壤调节植被指数 SAVI=(NIR-R)/(NIR+R+L)*(1+L)
    getSAVI(bands);
    //三波段梯度差植被指数 TGDVI
    getTGDVI(bands, lambda);
    //NDWI
    getNDWI(bands);
    //MNDWI
    getMNDWI(bands);
    //EWI
    getEWI(bands);
    //DBI 差值建筑覆盖指数
    getDBI(bands);
    //NDBI 归一化差值建筑用地指数
    getNDBI(bands);
    //NDBBI 归一化差值裸地与建筑用地指数
    getNDBBI(bands);
    //BSI 裸土指数
    getBSI(bands);

    //waitKey();


}

