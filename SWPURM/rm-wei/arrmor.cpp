#include "arrmor.h"
#include<algorithm.h>
#define DEBUG_LIGHT_VECTOR 0  //此宏会定义了关于灯条的各种参数的vector 用与看中间变量
#define DEBUG_IMAGE_SHOW_A 0//显示感兴趣区域的大小
#define SHOW_LIGHT_CONTOURS 0
#define SHOW_LIGHT_PUT_TEXT 0
#define DEBUG_IMG_ROI_SHOW_SRC 0
#define DEBUG_IMG_ROI_SHOW_SRC_ROI 0
#define DEBUG_ARRMOR 0
#define DEBUG_ARRMOR_PUTTEXT 0
#define DEBUG_DRAW_ALLARRMOR 0

#define TARGET_ARRMOR_RED  1

/**
 * 关于灯条描述类的一系列函数 
*/
bool screeningOfArmorrByConnect(Rect Num_Rect,ObjectType Type)
{
    Point centerRect=Point((Num_Rect.x+Num_Rect.width/2.0f),(Num_Rect.y+Num_Rect.height/2.0f));
    int errorX=0;
    int errorY=0;
    if(Type==SMALL_ARMOR)
    {
        errorX=fabs(centerRect.x-22);
        errorY=fabs(centerRect.y-20);
        if(Num_Rect.width<10)return 0;
        if(Num_Rect.width>40)return 0;
    }
    else if(Type==BIG_ARMOR)
    {
        errorX=fabs(centerRect.x-44);
        errorY=fabs(centerRect.y-20);
        if(Num_Rect.width<15)return 0;
        if(Num_Rect.width>60)return 0;
    }

    if(errorX>7)return 0;
    if(errorY>4)return 0;

   // if(digtisAreaRotia<0.35)return 0;
    return 1;
}
LightDescriptor::LightDescriptor()
{
    this->center.x=0;//初始化0,0点
    this->center.y=0;
    this->max=0;
    this->min=0;
    this->angle=0;
    this->area=0;
    this->sense=Lightleft;//  初始化为左
}
LightDescriptor::LightDescriptor(const cv::RotatedRect& another)
{
    if(another.size.width>another.size.height)// 灯条右向  /* /  */
    {
        this->sense=Lightright;
        this->angle=90+another.angle;
        this->max=another.size.width;
        this->min=another.size.height;
        this->center.x=another.center.x;
        this->center.y=another.center.y;

    }
    else                                    //  灯条左向  /* \  */
    {
        this->sense=Lightleft;
        this->angle=fabs(another.angle);
        this->max=another.size.height;
        this->min=another.size.width;
        this->center.x=another.center.x;
        this->center.y=another.center.y;
    }
}
LightDescriptor& LightDescriptor::operator =(const LightDescriptor& another)
{
    this->angle=another.angle;
    this->area=another.area;
    this->center.x=another.center.x;
    this->center.y=another.center.y;
    this->sense=another.sense;
    this->max=another.max;
    this->min=another.min;
    return *this;
}

cv::RotatedRect LightDescriptor::rotatedrect()
{
    float angleTemp=0;
    Size2f sizeTemp;
    if(this->sense==Lightright)
    {
        angleTemp=this->angle-90;
        sizeTemp.width=this->max;
        sizeTemp.height=this->min;
    }
    else
    {
        angleTemp=-(this->angle);
        sizeTemp.width=this->min;
        sizeTemp.height=this->max;
    }
    RotatedRect ro(Point2f(this->center),sizeTemp,angleTemp);
    return ro;
}


/**
 * 关于装甲板的函数
*/
ArmorDescriptor::ArmorDescriptor()
{
    this->angle=0;
    this->center.x=0;
    this->center.y=0;
    this->size=Size2f(0,0);
    this->armorType=UNKNOWN_ARMOR;
    this->robot=NoRotbor;
    this->Longest=0;
    this->Shortest=0;
    this->armorsense=Lightleft;
}

ArmorDescriptor:: ArmorDescriptor(const RotatedRect& another,ObjectType type,RobotType ro)
{
    this->angle=another.angle;
    this->center.x=another.center.x;
    this->center.y=another.center.y;
    this->size=another.size;
    this->armorType=type;
    this->robot=ro;
    if(another.size.width > another.size.height)
    {
        this->Longest=another.size.width;
        this->Shortest=another.size.height;
        this->armorsense=Lightright;
    }
    else
    {
        this->Longest=another.size.height;
        this->Shortest=another.size.width;
        this->armorsense=Lightleft;
    }
}
ArmorDescriptor& ArmorDescriptor::operator =(const ArmorDescriptor& another)
{
    this->angle=another.angle;
    this->center.x=another.center.x;
    this->center.y=another.center.y;
    this->size=another.size;
    this->armorType=another.armorType;
    this->robot=another.robot;
    this->Longest=another.Longest;
    this->Shortest=another.Shortest;
    this->armorsense=another.armorsense;
    return *this;
}

RotatedRect ArmorDescriptor::rotatedrect()
{
    RotatedRect temp(this->center,this->size,this->angle);
    return temp;
}

void ArmorDescriptor::setArmorrType(ObjectType set)
{
    this->armorType=set;
}

void ArmorDescriptor::setRobotType(RobotType set)
{
    this->robot=set;
}
void ArmorDescriptor::judgeArmorrType()
{
    float temp=Longest/Shortest;
    float arrmorangle=angle;
    if(fabs(arrmorangle)>45)arrmorangle+=90;
    else arrmorangle=fabs(arrmorangle);
    if(temp>3)
    {
        armorType=BIG_ARMOR;
    }
    else if(temp>2.6)
    {
        if(arrmorangle>4)armorType=BIG_ARMOR;
        else armorType=SMALL_ARMOR;
    }
    else if(temp>2.4)
    {
        if(arrmorangle>6)armorType=BIG_ARMOR;
        else armorType=SMALL_ARMOR;
    }
    else if(temp>2.2)
    {
        if(arrmorangle>9)armorType=BIG_ARMOR;
        else armorType=SMALL_ARMOR;
    }
    else armorType=SMALL_ARMOR;
}

/**
  *用line画框
*/
void drawRotatedangle(Mat& src,Point2f p[],Scalar Color[],int thickness)
{
    line(src,p[3],p[0],Color[0],thickness);//蓝
    line(src,p[0],p[1],Color[1],thickness);//绿
    line(src,p[1],p[2],Color[2],thickness);//红
    line(src,p[2],p[3],Color[3],thickness);
}
/**
 * 关于装甲板寻找的函数
*/
arrmor::arrmor()
{
    losed_counter=0;
    find_counter=0;
    _armorFindFlag=ARMOR_NO;
    roiImageSize=Size(0,0);

    _trackCounter=0;
    _isTracking=false;
    widthRatio=4;
    heightRatio=4;
    enableDigitsRecognize = true; //svm数字识别开关
}
arrmor::arrmor(Ptr<ml::SVM>& c, Ptr<ml::SVM>& d)
{
    losed_counter=0;
    find_counter=0;
    _armorFindFlag=ARMOR_NO;
    roiImageSize=Size(0,0);

    _trackCounter=0;
    _isTracking=false;
    widthRatio=4;
    heightRatio=4;
    enableDigitsRecognize = true; //svm数字识别开关
    this->SVM_Params=c;
    this->SVM_ArmorTypeParam = d;
}

void arrmor::judgeArrmorState()
{
    switch(_armorFindFlag)
    {
    case ARMOR_LOST:
    {
        if(losed_counter<=1)losed_counter++;
        else
        {
            losed_counter=0;
            _armorFindFlag=ARMOR_NO;
        }
        break;
    }
    case ARMOR_GLOBAL:
    {
        if(find_counter<=1)find_counter++;
        else
        {
            find_counter=0;
            _armorFindFlag=ARMOR_LOCAL;
        }
        break;
    }
    default :break;
    }
}

void arrmor::setImage(cv::Mat& set_src)
{
    ImageSize=set_src.size();
    vector<LightDescriptor> RotatedRectTemp;
    //确定需要处理的图像大小
    if((_armorFindFlag==ARMOR_NO)||(_armorFindFlag==ARMOR_GLOBAL))
    {
        src_roi=set_src;
        offset_roi_point.x=0;
        offset_roi_point.y=0;
    }
    else
    {
        if(_armorFindFlag==ARMOR_LOCAL)
        {
            targetArrmor2FindRoi=targetArrmor.rotatedrect();
            targetArrmor2FindRoi.size.width=targetArrmor2FindRoi.size.width*widthRatio;
            targetArrmor2FindRoi.size.height=targetArrmor2FindRoi.size.height*heightRatio;
            targetArrmor2FindRoi.center.x+=offset_roi_point.x;
            targetArrmor2FindRoi.center.y+=offset_roi_point.y;
        }
        cv::Rect tempSrcRoiRect=targetArrmor2FindRoi.boundingRect();//旋转矩形的最小包围Rect类型
        Rect srcRect(Point(0,0),set_src.size());
        Rect roiRect=tempSrcRoiRect;
        roiRect=srcRect&roiRect;

#if DEBUG_IMG_ROI_SHOW_SRC
    #if TARGET_ARRMOR_RED
        Point roiPoint[4];
        roiPoint[0]=Point(roiRect.x,roiRect.y);
        roiPoint[1]=Point(roiRect.x+roiRect.size().width,roiRect.y);
        roiPoint[2]=Point(roiRect.x+roiRect.size().width,roiRect.y+roiRect.size().height);
        roiPoint[3]=Point(roiRect.x,roiRect.y+roiRect.size().height);

        line(set_src,roiPoint[0],roiPoint[1],Scalar(255,0,0),4);
        line(set_src,roiPoint[1],roiPoint[2],Scalar(255,0,0),4);
        line(set_src,roiPoint[2],roiPoint[3],Scalar(255,0,0),4);
        line(set_src,roiPoint[3],roiPoint[0],Scalar(255,0,0),4);
        namedWindow("src");
        imshow("src",set_src);
    #else

        Point roiPoint[4];
        roiPoint[0]=Point(roiRect.x,roiRect.y);
        roiPoint[1]=Point(roiRect.x+roiRect.size().width,roiRect.y);
        roiPoint[2]=Point(roiRect.x+roiRect.size().width,roiRect.y+roiRect.size().height);
        roiPoint[3]=Point(roiRect.x,roiRect.y+roiRect.size().height);

        line(set_src,roiPoint[0],roiPoint[1],Scalar(0,0,255),4);
        line(set_src,roiPoint[1],roiPoint[2],Scalar(0,0,255),4);
        line(set_src,roiPoint[2],roiPoint[3],Scalar(0,0,255),4);
        line(set_src,roiPoint[3],roiPoint[0],Scalar(0,0,255),4);
        namedWindow("src");
        imshow("src",set_src);
    #endif

#endif
        src_roi=set_src(roiRect);
        offset_roi_point.x=roiRect.x;
        offset_roi_point.y=roiRect.y;
        offset_roi_point.x=offset_roi_point.x<0 ? 0:offset_roi_point.x;
        offset_roi_point.y=offset_roi_point.y<0 ? 0:offset_roi_point.y;
    }
    roiImageSize=src_roi.size();
   // cout << "获取的感兴趣区的大小为："<<roiImageSize<<endl;
#if DEBUG_IMAGE_SHOW_A
    cout << "获取的感兴趣区的大小为："<<roiImageSize<<endl;
#endif
    //开始进行预处理  重要  用红蓝--蓝红通道相减

    Mat src_roi_sub(src_roi.size(),CV_8UC1,Scalar(0));
    vector<Mat> src_roi_channels;
    split(src_roi,src_roi_channels);//  b g r
#if TARGET_ARRMOR_RED
    if(colorFlag)
    {
        subtract(src_roi_channels[0],src_roi_channels[2],src_roi_sub);//subtract  图像相减
    }
    else
    {
        subtract(src_roi_channels[2],src_roi_channels[0],src_roi_sub);//subtract  图像相减
    }
//    subtract(src_roi_channels[2],src_roi_channels[0],src_roi_sub);//subtract  图像相减
    threshold(src_roi_sub,src_roi_sub,40,255,THRESH_BINARY);//二值化
    Mat elementDilate = getStructuringElement(MORPH_ELLIPSE, Size(1, 3),Point(-1,-1));
    dilate(src_roi_sub,src_roi_sub,elementDilate);

#else
    subtract(src_roi_channels[0],src_roi_channels[2],src_roi_sub);
    threshold(src_roi_sub,src_roi_sub,80,255,THRESH_BINARY);
    Mat elementDilateBule=getStructuringElement(MORPH_RECT,Size(5,3),Point(-1,-1));
    dilate(src_roi_sub,src_roi_sub,elementDilateBule);
#endif
#if DEBUG_IMAGE_SHOW_A
#endif
    imshow("subImage",src_roi_sub);
#if DEBUG_IMAGE_SHOW_A
#endif
    /********************预处理结束**********************/
    vector<vector<Point>> lightCounters;
    vector<Vec4i> hierarchy;
    findContours(src_roi_sub,lightCounters,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);//RetrievalModes ContourApproximationModes
#if DEBUG_LIGHT_VECTOR
    vector<float> anglev;
    vector<float> widthv;
    vector<float> heightv;
    vector<int> areav;
#endif
    for(size_t i=0;i<lightCounters.size();i++)
    {
        float max_min=0;
        float widthTemp=0;
        float heightTemp=0;
        RotatedRect RectTemp=minAreaRect(lightCounters[i]);
        widthTemp=(RectTemp.size.width);
        heightTemp=(RectTemp.size.height);
#if DEBUG_LIGHT_VECTOR

        anglev.push_back(RectTemp.angle);//vector的size不是我理解的点集的个数
        widthv.push_back(RectTemp.size.width);
        heightv.push_back(RectTemp.size.height);
        putText(src_roi, to_string(i), RectTemp.center + Point2f(1,1) + offset_roi_point, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,255), 1);
        /*
        if(heightTemp!=0)
        {
            areav.push_back(widthTemp/heightTemp);
        }
        else
        {
            areav.push_back(0);
        }
        */
        float PointSize=widthTemp*heightTemp;
        // 1.先通过点集个数初步筛选出明显不符合条件的灯条
        areav.emplace_back(PointSize);


        if((PointSize<Param.Light_Point_min||PointSize>Param.Light_Point_max))continue;
        else if(PointSize<Param.Light_Point_min*2)
        {
            if(widthTemp>heightTemp)
            {
                float max_minTemp=widthTemp/heightTemp;
                if(max_minTemp<Param.Light_Max_Min2Points)continue;
            }
            else
            {
                float max_minTemp=heightTemp/widthTemp;
                if(max_minTemp<Param.Light_Max_Min2Points)continue;
            }
        }



#else
        float PointSize=widthTemp*heightTemp;// 点集的个数小于或者大于某个点数直接舍弃
        if((PointSize<Param.Light_Point_min||PointSize>Param.Light_Point_max))continue;
        else if(PointSize<Param.Light_Point_min*2)
        {
            if(widthTemp>heightTemp)
            {
                float max_minTemp=widthTemp/heightTemp;
                if(max_minTemp<Param.Light_Max_Min2Points)continue;
            }
            else
            {
                float max_minTemp=heightTemp/widthTemp;
                if(max_minTemp<Param.Light_Max_Min2Points)continue;
            }
        }

#endif
        // 2.通过灯条长宽比，倾斜角度筛选
        float Max_Light=0;
        float Min_Light=0;
        if(widthTemp>=heightTemp)
        {
            Max_Light=widthTemp;
            Min_Light=heightTemp;
            max_min=Max_Light/Min_Light;
            if(fabs(RectTemp.angle)<35||max_min<Param.Light_Normal_Max_Min2Points)continue;
        }
        else
        {
            Max_Light=heightTemp;
            Min_Light=widthTemp;
            max_min=Max_Light/Min_Light;
            if(fabs(RectTemp.angle)>55||max_min<Param.Light_Normal_Max_Min2Points)continue;
        }
#if DEBUG_LIGHT_VECTOR
        Point2f p[4];
        RectTemp.points(p);
        line(src_roi,p[3],p[0],Scalar(0,255,255),2);
        for(size_t i2=0;i2<3;i2++)
        {
            line(src_roi,p[i2],p[i2+1],Scalar(0,255,255),2);
        }
#endif

        LightDescriptor lightTemp(RectTemp);
        RotatedRectTemp.emplace_back(lightTemp);// 3
    }
#if DEBUG_LIGHT_VECTOR
    namedWindow("src_roitemp");
    imshow("src_roitemp",src_roi);
  //  waitKey(100);
  //  int a=100;
 //   int b=a;
#endif
    lightCountersRoRect=RotatedRectTemp;
}

//灯条排序模板
bool CmpLight(LightDescriptor R1,LightDescriptor R2)
{
    return R1.center.x<R2.center.x;
}
//旋转矩形排序模板
bool CmpRect(RotatedRect r1,RotatedRect r2)
{
    return r1.center.x<r2.center.x;
}
//装甲板排序模板
bool CmpArrmor(ArmorDescriptor A1,ArmorDescriptor A2)
{
    return A1.center.x<A2.center.x;
}

//装机板识别函数
ArmorFindFlag arrmor::ArrmorDection()
{
    //将灯条从左往右排序
    sort(lightCountersRoRect.begin(),lightCountersRoRect.end(),CmpLight);
    vector<ArmorDescriptor> arrrmor_vector;
    for(size_t i=0;i<lightCountersRoRect.size();i++)
    {
        float angleLeftLight=lightCountersRoRect[i].angle;//  旋转矩形的angle范围：[0,90)
        float Score=0;
        float ScoreTemp=0;
        int angleMark=-1;
        float lightRectMaxleftLight=lightCountersRoRect[i].max;
        sense_of_roRect rectTurnLeftLight=lightCountersRoRect[i].sense;
        sense_of_roRect rectTurnRightLightTemp=Lightleft;// 初始化为左
        // 自定义0度为灯条最理想的角度——竖直！！！
        for(size_t j=i+1;j<lightCountersRoRect.size();j++)
        {
            float angleRightLight=lightCountersRoRect[j].angle;
            float lightRectMaxRightLight=lightCountersRoRect[j].max;
            sense_of_roRect rectTurnRightLight=lightCountersRoRect[j].sense;
            float doubleLightAngleDifference=0;

  /********************开始匹配灯条**********************/
            if(rectTurnLeftLight!=rectTurnRightLight)
            {
                if(angleLeftLight>11) continue;// 不满足直接跳出
                doubleLightAngleDifference=angleLeftLight+angleRightLight;
            }
            else
            {
                if(rectTurnLeftLight==Lightleft)
                {
                    if((lightCountersRoRect[i].center.y-lightCountersRoRect[j].center.y)<-10)continue;//因为灯条是从左向右排序   因此可以由旋转矩形的角度确定两
                }                                                                                                          //灯条的中心坐标y值的范围，减少运算
                else
                {
                    if((lightCountersRoRect[i].center.y-lightCountersRoRect[j].center.y)>10)continue;
                }
                doubleLightAngleDifference=fabs(angleLeftLight-angleRightLight);
            }
            //旋转角度最大差
            bool rectAngleFlag=doubleLightAngleDifference<Param.angleij_diffMax ? false:true;
            if(rectAngleFlag)continue;

            //两灯条的中心点距离
            float lightRectMax= lightRectMaxleftLight>lightRectMaxRightLight ? lightRectMaxleftLight:lightRectMaxRightLight;
            float lightCenterDistance=cvex::distance(lightCountersRoRect[i].center,lightCountersRoRect[j].center);
            float disRatio=lightCenterDistance/lightRectMax;
            bool rectCenterLenFlagShort=disRatio>Param.disRatio_min ? false:true;//**************************************************************
            bool rectCenterLenFlagLong=disRatio<Param.disRatio_max ? false:true;
            if(rectCenterLenFlagShort||rectCenterLenFlagLong)continue;


            //两灯条的形状差别  *************************************************************************

            bool rectLenFlag=fabs(lightRectMaxleftLight-lightRectMaxRightLight)<(lightRectMax/Param.len_diff_kp) ? false:true;
            if(rectLenFlag)continue;

            //灯条中心点连线与灯条旋转角度的关系  **********************************************************
            float doubleLightCenterAngle=atan((lightCountersRoRect[i].center.y-lightCountersRoRect[j].center.y)/
                                            (lightCountersRoRect[i].center.x-lightCountersRoRect[j].center.x))/CV_PI*180;
            float doubleLightAngleaAve=0;
            if(rectTurnLeftLight!=rectTurnRightLight)
            {
                doubleLightAngleaAve=fabs(angleLeftLight-angleRightLight)/2;
            }
            else doubleLightAngleaAve=(angleLeftLight+angleRightLight)/2;
            doubleLightAngleaAve=fabs(doubleLightAngleaAve);
            doubleLightCenterAngle=fabs(doubleLightCenterAngle);
            bool angleijCenterFlag=fabs(doubleLightCenterAngle-doubleLightAngleaAve)<Param.angleij_center_diffMax ? false:true;
            if(angleijCenterFlag)continue;


            //滤掉两灯条中间存在灯条的装甲板
            //实现方式是：通过寻找装甲板的最高点和中间灯条最低点或者最低点和中间灯条的最高点进行比较。
            if((j-i)!=1)
            {
                bool breakflag=false;
                Point2f PointLeftLight[4];
                Point2f PointRightLight[4];
                lightCountersRoRect[i].rotatedrect().points(PointLeftLight);
                lightCountersRoRect[j].rotatedrect().points(PointRightLight);
                int maxy=PointLeftLight[0].y>PointRightLight[0].y ? PointLeftLight[0].y:PointRightLight[0].y;//y越下越
                int miny=PointLeftLight[2].y<PointRightLight[2].y ? PointLeftLight[2].y:PointRightLight[2].y;//
                int ave_center_y= (lightCountersRoRect[i].rotatedrect().center.y+lightCountersRoRect[j].rotatedrect().center.y)/2;
                //k表示两灯条的中间灯条
                for(size_t k=i+1;k<j;k++)
                {
                    Point2f PointMiddleLight[4];

                    lightCountersRoRect[k].rotatedrect().points(PointMiddleLight);
                    if(lightCountersRoRect[k].rotatedrect().center.y>ave_center_y)
                    {
                        if(maxy>PointMiddleLight[2].y)
                        {
                            breakflag=true;// 中间灯条的最低点和装甲板的最高点进行比较
                            break;
                        }
                    }
                    else
                    {
                         if(miny<PointMiddleLight[0].y)
                         {
                             breakflag=true;// 中间灯条的最高点和装甲板的最低点进行比较
                             break;
                         }
                    }
                }
                if(breakflag)continue;
            }
            float angleDifferenceScore=(Param.angleij_diffMax-doubleLightAngleDifference)/Param.angleij_diffMax*10;  // 16/10
            float lightSizeDifferenceScore=((lightRectMax/Param.len_diff_kp)-fabs(lightRectMaxleftLight-lightRectMaxRightLight))/(lightRectMax/Param.len_diff_kp)*8;
            float centerAngleDifferenceScore=(Param.angleij_center_diffMax-fabs(doubleLightCenterAngle-doubleLightAngleaAve))/Param.angleij_center_diffMax*6;
            //三个分数求和
            ScoreTemp=lightSizeDifferenceScore+centerAngleDifferenceScore+angleDifferenceScore;
            if(Score>ScoreTemp) continue;
            angleMark=j;
            Score=ScoreTemp;
            rectTurnRightLightTemp=rectTurnRightLight;
        }
        //某一个灯条匹配完成
        if(angleMark!=(-1))//不等于-1表示有符合条件的装甲板
        {
            RotatedRect arrmorTemp;
            Point2f Point4_i[4];
            Point2f Point4_j[4];
            //Point PointArrmor[4];
            vector<Point2f> PointArrmor;
            RotatedRect lightRectTempi=lightCountersRoRect[i].rotatedrect();
            RotatedRect lightRectTempj=lightCountersRoRect[angleMark].rotatedrect();
            lightRectTempi.points(Point4_i);
            lightRectTempj.points(Point4_j);
            if(rectTurnLeftLight==Lightleft)
            {
                PointArrmor.emplace_back(Point4_i[0]);
                PointArrmor.emplace_back(Point4_i[1]);
            }
            else
            {
                PointArrmor.emplace_back(Point4_i[1]);
                PointArrmor.emplace_back(Point4_i[2]);
            }
            if(rectTurnRightLightTemp==Lightleft)
            {
                PointArrmor.emplace_back(Point4_j[2]);
                PointArrmor.emplace_back(Point4_j[3]);
            }
            else
            {
                PointArrmor.emplace_back(Point4_j[3]);
                PointArrmor.emplace_back(Point4_j[0]);
            }
            //根据两灯条自身的长度与两灯条的距离关系筛查灯条
            arrmorTemp=minAreaRect(PointArrmor);
            float arrmorLenMaxTemp=0;
            float arrmorLenMinTemp=0;
            ArmorDescriptor targetArrmorTemp(arrmorTemp);
            if(targetArrmorTemp.armorsense==Lightright)
            {
                arrmorLenMaxTemp=arrmorTemp.size.width;
                arrmorLenMinTemp=arrmorTemp.size.height;
                targetArrmorTemp.size.height=targetArrmorTemp.size.height*2;
            }
            else
            {
                arrmorLenMinTemp=arrmorTemp.size.width;
                arrmorLenMaxTemp=arrmorTemp.size.height;
                targetArrmorTemp.size.width=targetArrmorTemp.size.width*2;
            }
            float arrmorLenMaxMinTemp=arrmorLenMaxTemp/arrmorLenMinTemp;
            bool arrmor_LenFlag1=arrmorLenMinTemp<2 ? true:false;
            bool arrmor_LenFlag2=arrmorLenMaxMinTemp>5.3 ? true:false;
            bool arrmor_LenFlag3=arrmorLenMaxMinTemp<1.5 ? true:false;
            if((arrmor_LenFlag1||arrmor_LenFlag2)||arrmor_LenFlag3)continue;
            judgeArmorrType(targetArrmorTemp)            ;
            float len=0                                  ;
            getLightLen(PointArrmor,len)                 ;
            targetArrmorTemp.lightLen=len                ;
            arrrmor_vector.emplace_back(targetArrmorTemp);
#if DEBUG_ARRMOR
            putText(src_roi, to_string(Score), arrmorTemp.center + Point2f(10,0) + offset_roi_point, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,255), 1);
            Point2f targetArrmorPoint[4];
            targetArrmorTemp.points(targetArrmorPoint);
            line(src_roi,targetArrmorPoint[3],targetArrmorPoint[0],Scalar(0,0,255),2);
            for(size_t i2=0;i2<3;i2++)
            {
                line(src_roi,targetArrmorPoint[i2],targetArrmorPoint[i2+1],Scalar(0,0,255),2);
            }
#endif
        }
#if SHOW_LIGHT_CONTOURS
        maxScore.emplace_back(Score);//初始化
        maxScoreMark.emplace_back(angleMark);//表示没有配对的灯条
#endif
#if SHOW_LIGHT_PUT_TEXT
        putText(src_roi, to_string(i), lightCountersRoRect[i].center + Point2f(10,0) + offset_roi_point, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,255), 1);
#endif
#if SHOW_LIGHT_CONTOURS

        Point2f PointFour[4];
        RNG rng;
        uchar Bscalar=rng.uniform(0,255);
        uchar Gscalar=rng.uniform(0,255);
        uchar Rscalar=rng.uniform(0,255);
        lightCountersRoRect[i].points(PointFour);
        line(src_roi,PointFour[3],PointFour[0],Scalar(Bscalar,Gscalar,Rscalar),2);
        for(size_t i=0;i<3;i++)
        {
            line(src_roi,PointFour[i],PointFour[i+1],Scalar(Bscalar,Gscalar,Rscalar),2);
        }
#endif
    }
    //装甲板寻找结束
    //装甲板筛选
    ArmorDescriptor targetArrmorTemp;
    vector<ArmorDescriptor> digtisArmorTemp;
    vector<int> ArmorNum;
    if(!arrrmor_vector.empty())
    {
        Mat dstArmorGray ;
        Mat dstBinArmor  ;
        Mat dstArmor     ;
        Mat dstArmorRed  ;
        Mat dstArmorBule ;
        if(enableDigitsRecognize==true)  //数字识别开关
        {
            for(size_t i=0;i<arrrmor_vector.size();i++)
            {
                Point2f ArmorPoint[4];
                arrrmor_vector[i].points(ArmorPoint);
                Point2f threeVertexPoint[3];
                if(arrrmor_vector[i].armorsense==Lightright)
                {

                    threeVertexPoint[0]=ArmorPoint[1];
                    threeVertexPoint[1]=ArmorPoint[2];
                    threeVertexPoint[2]=ArmorPoint[3];
                }
                else
                {
                    threeVertexPoint[0]=ArmorPoint[2];
                    threeVertexPoint[1]=ArmorPoint[3];
                    threeVertexPoint[2]=ArmorPoint[0];
                }
                if(arrrmor_vector[i].armorType==BIG_ARMOR)
                {
                    dstArmor=Mat(Size(88,40),src_roi.type(),Scalar::all(0));//arrrmor_vector[i].Shortest
                }
                else if(arrrmor_vector[i].armorType==SMALL_ARMOR)
                {
                    dstArmor=Mat(Size(44,40),src_roi.type(),Scalar::all(0));//arrrmor_vector[i].Shortest
                }

                Point2f threeDstImagePoint[3];
                threeDstImagePoint[0]=Point2f(0,0);
                threeDstImagePoint[1]=Point2f(dstArmor.size().width,0);
                threeDstImagePoint[2]=Point2f(dstArmor.size().width,dstArmor.size().height);
                Mat M=getAffineTransform(threeVertexPoint,threeDstImagePoint);//由三点获取放射变换的模板矩阵
                warpAffine(src_roi,dstArmor,M,dstArmor.size());//仿射变换
                cvtColor(dstArmor,dstArmorGray,CV_BGR2GRAY);
                equalizeHist(dstArmorGray,dstArmorGray);

                vector<Mat> dstArmorChannels;
                split(dstArmor,dstArmorChannels);

                if(armorColor==false)
                {
                    subtract(dstArmorChannels[2],dstArmorChannels[0],dstArmorRed);//subtract  图像相减
                    threshold(dstArmorRed,dstArmorRed,40,255,THRESH_BINARY_INV);//二值化
                    threshold(dstArmorGray,dstBinArmor,200,255,THRESH_BINARY);
                    dstBinArmor=dstBinArmor&dstArmorRed;
                }
                else
                {
                    subtract(dstArmorChannels[0],dstArmorChannels[2],dstArmorBule);//subtract  图像相减
                    threshold(dstArmorBule,dstArmorBule,20,255,THRESH_BINARY_INV);//二值化
                    threshold(dstArmorGray,dstBinArmor,160,255,THRESH_BINARY);
                    dstBinArmor=dstBinArmor&dstArmorBule;

                }


                //通过找连通区域的方法筛选
                Mat labels,stats,centroids;
                int num=connectedComponentsWithStats(dstBinArmor,labels,stats,centroids);//num-1为连通区域的个数
                int maxLabel=0;
                int maxArea=0;
                //找出最大连通区域
                for(int j=1;j<num;j++)
                {
                    if(maxArea<(stats.at<int>(j,CC_STAT_AREA)))
                    {
                        maxLabel=j;//记录label
                        maxArea=stats.at<int>(j,CC_STAT_AREA);
                    }
                }
                //通过连通区域筛选装甲板
                 Point numLeftTopPoint=Point(stats.at<int>(maxLabel,CC_STAT_LEFT),stats.at<int>(maxLabel,CC_STAT_TOP));
                 Point numRightBottomPoint=Point(numLeftTopPoint.x+stats.at<int>(maxLabel,CC_STAT_WIDTH),numLeftTopPoint.y+stats.at<int>(maxLabel,CC_STAT_HEIGHT));
                 Rect numRect(Rect(numLeftTopPoint,numRightBottomPoint));
                 bool temp=screeningOfArmorrByConnect(numRect,arrrmor_vector[i].armorType);
                 if(!temp)
                 {
    //                //滤波得到只有数字的区域
    //                Mat connectDigtisDst(dstBinArmor.size(),dstBinArmor.type(),Scalar::all(0));
    //                drawMaxConnect(connectDigtisDst,labels,maxLabel);
    //                imshow("error",connectDigtisDst);

                 }
                 else
                 {
    //装甲板最大连通区域是，正中位置，是真的装甲板
                    digtisArmorTemp.emplace_back(arrrmor_vector[i]);
    //滤波得到只有数字的区域
                    Mat connectDigtisDst(dstBinArmor.size(),dstBinArmor.type(),Scalar::all(0));
                    drawMaxConnect(connectDigtisDst,labels,maxLabel);
                    imshow("asdwa",connectDigtisDst);
//                    if(waitKey(0) == 'q')
//                    {
//                        string str = "/home/weishibo/picture/4/";
//                        string s = ".png";
//                        srand((unsigned)time(NULL));
//                        string res = str + to_string(rand())+s;
//                        imwrite(res,connectDigtisDst);
//                    }
    //装甲板数字
                    int AN;
    //svm数字识别
                    mySvmPredict(connectDigtisDst,AN);
                    ArmorNum.emplace_back(AN);
                 }
             }
        }
        else
        {
            digtisArmorTemp=arrrmor_vector;
        }


#if DEBUG_DRAW_ALLARRMOR

            imshow("1",dstBinArmor);
            imshow("name",dstMorpArmor);
            imshow("dstArmor",dstArmor);
           // cout << name << M <<endl;
//            Scalar armorTempColor[4]={Scalar(0,255,255),Scalar(0,255,255),Scalar(0,255,255),Scalar(0,255,255)};
//            drawRotatedangle(src_roi,ArmorPoint,armorTempColor,2);
        }
#endif
        if(!digtisArmorTemp.empty())
        {
            //将装甲板按照中心坐标x 从小到大排序
            sort(digtisArmorTemp.begin(),digtisArmorTemp.end(),CmpArrmor);
            //滤掉反光装甲板的影响
            if(digtisArmorTemp.size()>1)
            {
                for(size_t i=1;i<digtisArmorTemp.size();i++)
                {

                    if((digtisArmorTemp[i].center.x-digtisArmorTemp[i-1].center.x)<(digtisArmorTemp[i-1].Longest/2))
                    {
                        if(digtisArmorTemp[i].center.y<digtisArmorTemp[i-1].center.y)// y小的是真的y大的是反光的
                        {
                            digtisArmorTemp[i-1]=digtisArmorTemp[i];
                        }
                        else
                        {
                            digtisArmorTemp[i]=digtisArmorTemp[i-1];
                        }
                    }
                }
            }

            //只是单纯的从装甲板与图像中心距离的关系找出最合适的装甲板
            //后面筛选出目标装甲板的代码还需要优化
            int mask=0;
            float targetArrmorDistanceTemp=0;
            Point2f srcCenter(ImageSize.width/2,ImageSize.height/2);
            Point2f targetArrmorCenterTemp(digtisArmorTemp[mask].center.x+offset_roi_point.x,
                                           digtisArmorTemp[mask].center.y+offset_roi_point.y);
            targetArrmorDistanceTemp=cvex::distance(targetArrmorCenterTemp,srcCenter);
            for(size_t i=1;i<digtisArmorTemp.size();i++)
            {
                Point2f arrmorCenterTemp(digtisArmorTemp[i].center.x+offset_roi_point.x,
                                         digtisArmorTemp[i].center.y+offset_roi_point.y);
                float arrmorDistanceTemp=cvex::distance(arrmorCenterTemp,srcCenter);


                //与摄像头中心距离
                if((targetArrmorDistanceTemp-arrmorDistanceTemp)>=0)
                {
                    mask=i;
                    targetArrmorDistanceTemp=arrmorDistanceTemp;
                }

                //画出找到的装甲板
                Point2f allArmorPoints[4];
                Scalar allArmorPointsColor[4]={Scalar(255,0,0),Scalar(0,255,0),Scalar(0,0,255),Scalar(255,0,255)};
                digtisArmorTemp[i].points(allArmorPoints);
                drawRotatedangle(src_roi,allArmorPoints,allArmorPointsColor,2);
            }
            targetArrmorTemp=digtisArmorTemp[mask];

            if(enableDigitsRecognize==true)
            {
                int digtis=ArmorNum[mask];
               // cout << "target=" <<digtis<<endl;
            }


            Point2f targetArrmorPoint[4];

            targetArrmorTemp.points(targetArrmorPoint);
            //求出目标装甲板的中心像素坐标
            Points_coordinates.x = (targetArrmorPoint[0].x+targetArrmorPoint[1].x+targetArrmorPoint[2].x+targetArrmorPoint[3].x)/4;
            Points_coordinates.y = (targetArrmorPoint[0].y+targetArrmorPoint[1].y+targetArrmorPoint[2].y+targetArrmorPoint[3].y)/4;
            h_light = targetArrmorTemp.lightLen;
            //画出目标装甲板

          //  Scalar targetArmorColor[4]={Scalar(255,0,0),Scalar(0,255,0),Scalar(0,0,255),Scalar(255,0,255)};//彩色
            //储存目标装甲板信息到算法对象中

            // algorithms.get_Point(targetArrmorPoint[0],targetArrmorPoint[1],targetArrmorPoint[2],targetArrmorPoint[3],targetArrmorTemp.lightLen);

            Scalar targetArmorColor[4]={Scalar(255,0,0),Scalar(255,0,0),Scalar(255,0,0),Scalar(255,0,0)};
            drawRotatedangle(src_roi,targetArrmorPoint,targetArmorColor,2);
            namedWindow("endsrc");
            imshow("endsrc",src_roi);

    #if DEBUG_ARRMOR_PUTTEXT
            putText(src_roi, to_string(targetArrmorTemp.Longest/targetArrmorTemp.Shortest), targetArrmorTemp.center + Point2f(10,0) , FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255,255,0), 1);
            if(targetArrmorTemp.armorType==BIG_ARMOR)
            {
                putText(src_roi, "big", targetArrmorTemp.center + Point2f(10,60) , FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255,255,0), 1);
            }
            else
            {
                putText(src_roi, "small", targetArrmorTemp.center + Point2f(10,80) , FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255,255,0), 1);
            }

    #endif
            targetArrmor=targetArrmorTemp;// 存储目标装甲板

            recrodArmorStatus(true);

        }
        else  //没有找到装甲板
        {
            recrodArmorStatus(false);
            targetArrmor.armorType=UNKNOWN_ARMOR;
        }
    #if DEBUG_IMG_ROI_SHOW_SRC_ROI
        namedWindow("src_roi");
        imshow("src_roi",src_roi);
        cout << "_trackCounter="<< _trackCounter <<endl<< "losed_counter="<<losed_counter<<endl<<"find_counter"<<find_counter <<endl;
    #endif


    }
    else  //没有找到装甲板
    {
        recrodArmorStatus(false);
        targetArrmor.armorType=UNKNOWN_ARMOR;
    }

    return _armorFindFlag;
}




// 直方图显示
void arrmor::histMaker(Mat& src_hist)
{
    int channels=src_hist.channels();
    if(channels==3)
    {
        long hist[3][256]={0};
        for(size_t i=0;i<(unsigned int)(src_hist.rows);i++)
        {
            Vec3b* ptr=src_hist.ptr<Vec3b>(i);
            for(size_t j=0;j<(unsigned int)(src_hist.cols);j++)
            {
                hist[0][ptr[j][0]]++;
                hist[1][ptr[j][1]]++;
                hist[2][ptr[j][2]]++;
            }
        }

        Mat histMatB(Size(800,1000),CV_8UC3,Scalar(0,0,0));
        Mat histMatG(Size(800,1000),CV_8UC3,Scalar(0,0,0));
        Mat histMatR(Size(800,1000),CV_8UC3,Scalar(0,0,0));
        for(size_t i=10;i<778;i+=3)
        {
            int BPointy=hist[0][(int)(i/3)]/8;
            BPointy= BPointy>999 ? 999:BPointy;
            int GPointy=hist[1][(int)(i/3)]/8;
            GPointy= GPointy>999 ? 999:GPointy;
            int RPointy=hist[2][(int)(i/3)]/8;
            RPointy= RPointy>999 ? 999:RPointy;
            line(histMatB,Point(i,(histMatB.size().height-1)),Point(i,(histMatB.size().height-BPointy-1)),Scalar(255,0,0));
            line(histMatG,Point(i,(histMatG.size().height-1)),Point(i,(histMatG.size().height-GPointy-1)),Scalar(0,255,0));
            line(histMatR,Point(i,(histMatR.size().height-1)),Point(i,(histMatR.size().height-RPointy-1)),Scalar(0,0,255));
        }
        namedWindow("histB",WINDOW_NORMAL);
        imshow("histB",histMatB);
        namedWindow("histG",WINDOW_NORMAL);
        imshow("histG",histMatG);
        namedWindow("histR",WINDOW_NORMAL);
        imshow("histR",histMatR);
    }
    else if(channels==1)
    {
        cout <<"此图像通道数为1，请输入通道数为3的图像"<< endl;
    }

}
//净化图片，得到只有数字区域的装甲板
void  arrmor::drawMaxConnect(Mat& out,Mat& labels,int maxLabel)
{
    for(int y=0;y<out.rows;y++)
    {
        for(int x=0;x<out.cols;x++)
        {
            if(labels.ptr<int>(y)[x]==maxLabel)
            {
              out.ptr<uchar>(y)[x]=255;
            }
            else
            {
              out.ptr<uchar>(y)[x]=0;
            }
        }
    }
}
//svm预测函数
void arrmor::mySvmPredict(Mat& src,int& armorNum)
{
    Rect ShowRoi=Rect(src.size().width/4,0,src.size().width/2,src.size().height);     //定义rect of roi
    Mat connectDigtisDstShow=src(ShowRoi);                                            //只取中间1/3部分
    imshow("predict",connectDigtisDstShow);
    /*Mat trainDate;
    Mat reshapeDate;
    connectDigtisDstShow.convertTo(reshapeDate,CV_32FC1);                             //转换预测数据类型位32f1
    trainDate=reshapeDate.reshape(1,1);                                               //将预测数据整合成行向量
    int predict=SVM_Params->predict(trainDate);                                       //svm数据预测
    armorNum=predict;*/                                                                 //armorNum为预测的数
}

int arrmor::mySvmArmorTypePredict(float ratio, float angle)
{
    if(angle<-45)angle+=90;
    else if(angle<0)angle=-angle;
    Mat predictData=(Mat_<float>(1,2) << ratio,angle);
//    int result=SVM_ArmorTypeParam->predict(predictData);
    return 1;
}

void arrmor::recrodArmorStatus(bool isFoundArmor)
{
    if(isFoundArmor)
    {
        _trackCounter++;
        if(_trackCounter==3000)//每3000帧执行一次全局扫描
        {
            _trackCounter=0;
            _armorFindFlag=ARMOR_GLOBAL;
        }

        switch(_armorFindFlag)//通过装甲板的状态确定用何种方式寻找装甲板
        {
            case ARMOR_LOST://上一帧是丢失状态下继续在最近找到装甲板的位置找装甲板
            {
                _armorFindFlag=ARMOR_LOCAL;
                break;
            }
            case ARMOR_NO://上一帧是没有找到装甲板的状态下，在全图中寻找装甲板
            {
                _armorFindFlag=ARMOR_GLOBAL;

                break;
            }
            default :
            {

                break;
            }
        }
    }
    else
    {
        if(_armorFindFlag==ARMOR_LOCAL)//上一帧是局部寻找，则标记为丢失状态
        {
            _armorFindFlag=ARMOR_LOST;
        }
        else if(_armorFindFlag==ARMOR_GLOBAL)//上一帧是全局寻找，则标记为没有找到装甲板的状态状态
        {

            _armorFindFlag=ARMOR_NO;
        }
    }
}
void arrmor::judgeArmorrType(ArmorDescriptor &a)
{
    float ratio=a.Longest/a.Shortest;
    int r=mySvmArmorTypePredict(ratio,a.angle);
    if(r==1)
    {
        a.setArmorrType(SMALL_ARMOR);
    }
    else
    {
        a.setArmorrType(BIG_ARMOR)  ;
    }
}
void  arrmor::getLightLen(vector<Point2f> &lightPoint2fs, float &len)
{
    float leftLength =cvex::distance(lightPoint2fs[0],lightPoint2fs[1]);

    float rightlength=cvex::distance(lightPoint2fs[2],lightPoint2fs[3]);

    len=leftLength > rightlength ? leftLength : rightlength;
}
