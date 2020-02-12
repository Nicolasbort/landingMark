#include<opencv2/opencv.hpp>
#include<iostream>

using namespace cv;
using namespace std;


//limiares de saturacao e valor
int MINSAT = 50; //40
int MAXSAT = 255;
int MINVAL = 50; //40
int MAXVAL = 255;

//limiares da cor amarela
int YELLOW = 25; //30
int DYELLOW = 15; //25
int MINYELLOW = YELLOW - DYELLOW;
int MAXYELLOW = YELLOW + DYELLOW;
int ARR_MAXYELLOW[3] = {MAXYELLOW, MAXSAT, MAXVAL};
int ARR_MINYELLOW[3] = {MINYELLOW, MINSAT, MINVAL};

//limiares da cor azul
int BLUE = 120; //110
int DBLUE = 20; //25
int MINBLUE = BLUE - DBLUE;
int MAXBLUE = BLUE + DBLUE;
int ARR_MAXBLUE[3] = {MAXBLUE, MAXSAT, MAXVAL};
int ARR_MINBLUE[3] = {MINBLUE, MINSAT, MINVAL};

//parametros de filtros
int GAUSSIAN_FILTER = 3;
int KERNEL_RESOLUTION = 7;

//dimensoes da base real
float ARESTA = 500.0f; //aresta da base (em mm)
float RAIO = 200.0f; //raio do centro da base 
int RESOLUTION = 50;


bool IMAGEM = false;
bool VIDEO = true;


class LandingMark
{
public:

	Mat imagem;

	Mat rot_vec, trans_vec;

	int x, y, z;

	int rows, cols;

	int focal_lenght;

	int center[2];

	Mat camera_matrix, dist_coeffs, kernel;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;


	LandingMark()
	{
		this->rot_vec = Mat::zeros(4, 1, CV_32F);
		this->trans_vec = Mat::zeros(3, 1, CV_32F);

		this->x = 0;
		this->y = 0;
		this->z = 0;

		this->kernel = Mat::ones(KERNEL_RESOLUTION, KERNEL_RESOLUTION, CV_8U);
	}


	void CamParam(Mat img)
	{
		this->rows = img.rows;
		this->cols = img.cols;

		this->focal_lenght = this->rows;

		this->center[0] = (this->rows/2);
		this->center[1] = (this->cols/2);

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


	Mat processImage()
	{

        cvtColor(this->imagem, this->imagem, COLOR_BGR2HSV);

		Mat quadrado_azul = this->imlimiares(this->imagem, ARR_MINBLUE, ARR_MAXBLUE);

		morphologyEx(quadrado_azul, quadrado_azul, MORPH_CLOSE, this->kernel, Point(-1,-1), 1);

		findContours(quadrado_azul, this->contours, this->hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0,0));

		return quadrado_azul;
	}


	Mat drawEllipse()
	{
		Mat img_processada = this->processImage();

		for(size_t i = 0; i < this->contours.size(); i++)
        {
            size_t count = this->contours[i].size();

            if (count < 6)
                break;

            Mat pointsf;
            Mat(this->contours[i]).convertTo(pointsf, CV_32F);
            RotatedRect box = fitEllipse(pointsf);

            if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*30 )
                continue;


            ellipse(img_processada, box.center, box.size*0.5f, box.angle, 0, 360, 150, 2, CV_AA);


            //Centro da elipse
            cout << "X: " << box.center.x << "Y: " << box.center.y << endl;
        }

		return img_processada;
	}


	Mat imfill(Mat img)
	{


		morphologyEx(img, img, MORPH_CLOSE, this->kernel, Point(-1,-1), 3);

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


	Mat imlimiares(Mat hsv, int hsvMin[3], int hsvMax[3])
	{
		Mat hsvtresh;

		inRange(hsv, Scalar(hsvMin[0], hsvMin[1], hsvMin[2]), Scalar(hsvMax[0], hsvMax[1], hsvMax[2]), hsvtresh);

		//hsvtresh = this->imfill(hsvtresh);

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



int main()
{

	if (IMAGEM)
	{
		//////////// COM IMAGEM ///////////
		

		LandingMark mark;

		Mat img = imread("imagem.jpeg");

		if (img.empty())
		{
			cout << "Imagem vazia!";
			return 0;
		}

		mark.setImage(img);

		img = mark.processImage();

		imshow("", img);

		waitKey();
	}
	else if(VIDEO)
	{
		/////////////// COM VIDEO //////////////


		LandingMark mark;

		VideoCapture cap("mark.mp4");

		if(!cap.isOpened()){
			cout << "Error opening video stream or file" << endl;
			return -1;
		}

		Mat frame;

		while (true)
		{
			cap >> frame;

			mark.setImage(frame);

			frame = mark.drawEllipse();

			imshow("Frame", frame);

			waitKey(20);
		}
	}
}
