#include<opencv2/opencv.hpp>
#include<iostream>
#include<tuple>


using namespace cv;
using namespace std;

//limiares de saturacao e valor
int MINSAT = 40;
int MAXSAT = 255;
int MINVAL = 40;
int MAXVAL = 255;;

//limiares da cor amarela
int YELLOW = 30;
int DYELLOW = 25;
int MINYELLOW = YELLOW - DYELLOW;
int MAXYELLOW = YELLOW + DYELLOW;

//limiares da cor azul
int BLUE = 110;
int DBLUE = 25;
int MINBLUE = BLUE - DBLUE;
int MAXBLUE = BLUE + DBLUE;

//parametros de filtros
int GAUSSIAN_FILTER = 3;
int KERNEL_RESOLUTION = 7;

//dimensoes da base real
float ARESTA = 500.0f; //aresta da base (em mm)
float RAIO = 200.0f; //raio do centro da base 
int RESOLUTION = 50;


class LandingMark
{
public:

	Mat imagem;

	Mat rot_vec, trans_vec;

	int x, y, z;

	int rows, cols;

	int focal_lenght;

	tuple<int, int> center;

	Mat camera_matrix, dist_coeffs;


	LandingMark()
	{
		this->rot_vec = Mat::zeros(4, 1, CV_32F);
		this->trans_vec = Mat::zeros(3, 1, CV_32F);

		this->x = 0;
		this->y = 0;
		this->z = 0;
	}


	void CamParam(Mat img)
	{
		this->rows = img.rows;
		this->cols = img.cols;

		this->focal_lenght = this->rows;

		this->center = make_tuple(this->rows/2, this->cols/2);

		this->camera_matrix = (Mat_<double>(3,3) << 
			this->focal_lenght, 0, this->rows/2,
			0, this->focal_lenght, this->cols/2,
			0, 0, 1);

		this->dist_coeffs = Mat::zeros(3, 1, CV_32F);
	}


	void setImage(Mat img)
	{
		GaussianBlur(img, img, Size(GAUSSIAN_FILTER, GAUSSIAN_FILTER), 0);

		this->CamParam(img);

		this->imagem = img;
	}


	void processImage()
	{
		Mat hsv;

		Mat kernel = Mat::ones(KERNEL_RESOLUTION, KERNEL_RESOLUTION, CV_8U);
	}


	Mat imfill(Mat img)
	{
		vector<vector<Point> > contours;
  		vector<Vec4i> hierarchy;

		Mat kernel = Mat::ones(KERNEL_RESOLUTION, KERNEL_RESOLUTION, CV_8U);

		morphologyEx(img, img, MORPH_CLOSE, kernel, Point(-1,-1), 3);

		findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0,0));


	    vector<vector<Point> >hull( contours.size() );
	    for( size_t i = 0; i < contours.size(); i++ )
	    {
	        convexHull( contours[i], hull[i] );
	    }


  		for( size_t i = 0; i< contours.size(); i++ )
    	{
    		//Scalar color(255, 255, 255);
	        drawContours( img, contours, 0, 255, -1);
        	drawContours( img, hull, 0, 255 , -1);
    	}

    	return img;
	}


	Mat imlimiares(Mat hsv, tuple<int, int, int> hsvMin, tuple<int, int, int> hsvMax)
	{
		Mat hsvtresh;

		int hsvMin1, hsvMin2, hsvMin3;
		int hsvMax1, hsvMax2, hsvMax3;
		
		tie(hsvMin1, hsvMin2, hsvMin3) = hsvMin;
		tie(hsvMax1, hsvMax2, hsvMax3) = hsvMax;

		inRange(hsv, Scalar(hsvMin1, hsvMin2, hsvMin3), Scalar(hsvMax1, hsvMax2, hsvMax3), hsvtresh);

		hsvtresh = this->imfill(hsvtresh);

		return hsvtresh;
	}


	void show()
	{
		if (this->imagem.empty())
		{
			cout << "Sem imagem";
			return;
		}

		imshow("this->image", this->imagem);
	}
};



Mat imfilled(Mat img)
{
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	inRange(img, Scalar(MINYELLOW, MINSAT, MINVAL), Scalar(MAXYELLOW, MAXSAT, MAXVAL), img);	

	Mat kernel = Mat::ones(KERNEL_RESOLUTION, KERNEL_RESOLUTION, CV_8U);

	morphologyEx(img, img, MORPH_CLOSE, kernel, Point(-1,-1), 3);

	findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0,0));


    vector<vector<Point> >hull( contours.size() );
    for( size_t i = 0; i < contours.size(); i++ )
    {
        convexHull( contours[i], hull[i] );
    }


		for( size_t i = 0; i< contours.size(); i++ )
	{
		//Scalar color(255, 255, 255);
        drawContours( img, contours, 0, 255, -1);
    	drawContours( img, hull, 0, 255 , -1);
	}

	return img;
}


Mat getCanny(Mat img)
{

	vector<vector<Point> > contours;
  	vector<Vec4i> hierarchy;

	Canny(img, img, 100, 200);

	findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0,0));

 	for( size_t i = 0; i< contours.size(); i++ )
    {
    	Scalar color = Scalar( 255, 255, 255);
    	drawContours( img, contours, 0, 255, -1);
    }
	return img;
}

int main()
{
	//LandingMark mark;


	//Mat img = imread("imagem.jpeg");

	/*if (img.empty())
	{
		cout << "Imagem vazia!";
		return 0;
	}*/

	//mark.setImage(img);



	//img = imfilled(img);

	//cout << "\nCamera Matrix: \n" << mark.dist_coeffs << endl;
	
	//imshow("img", img);



	VideoCapture cap("mark.mp4");

	if(!cap.isOpened()){
    	cout << "Error opening video stream or file" << endl;
    	return -1;
  	}

	Mat frame, frameMod;

	while (true)
	{
		cap >> frameMod;

		frameMod = getCanny(frameMod);

		imshow("Frame", frameMod);

		waitKey(100);
	}

	//waitKey(0);

}