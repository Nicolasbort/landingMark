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

	int centerX, centerY;

	Mat camera_matrix, dist_coeffs, kernel;

	int majorEllipseWidth, majorEllipseHeight;
	RotatedRect majorEllipse;

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

		this->majorEllipseWidth = 0;
		this->majorEllipseHeight = 0;
	}


	void CamParam(Mat img)
	{
		this->rows = img.rows;
		this->cols = img.cols;

		this->focal_lenght = this->rows;

		this->centerX = img.size().width/2;
		this->centerY = img.size().height/2;

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
		Mat temp_img = this->imagem.clone();

        cvtColor(this->imagem, temp_img, COLOR_BGR2HSV);

		Mat img_quadrado_azul = this->imlimiares(temp_img, ARR_MINBLUE, ARR_MAXBLUE);

		morphologyEx(img_quadrado_azul, img_quadrado_azul, MORPH_CLOSE, this->kernel, Point(-1,-1), 2);

		findContours(img_quadrado_azul, this->contours, this->hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0,0));
	}


	bool drawEllipse()
	{
		this->processImage();
;
		size_t countoursSize;
		Mat pointsf;
		RotatedRect box;
		bool success = false;

		for(size_t i = 0; i < this->contours.size(); i++)
        {
            countoursSize = this->contours[i].size();

            if (countoursSize < 5)
                continue;

            Mat(this->contours[i]).convertTo(pointsf, CV_32F);
            box = fitEllipse(pointsf);
			
			// Verifica se a elipse tem eixos maiores que 120
			if ( box.size.width > 120 && box.size.height > 120 )
			{
				// Pega a maior elipse da imagem
				if ( box.size.width > this->majorEllipseWidth && box.size.height > this->majorEllipseHeight)
				{
					this->majorEllipse = box;
					success = true;
				}
			}
			else
			{
				success = false;
			}
        }


		if ( success )
		{
			ellipse(this->imagem, this->majorEllipse.center, this->majorEllipse.size*0.5f, this->majorEllipse.angle, 0, 360, 180, 2);
		}

		return success;
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

	const char* name_video = "mark.mp4";
	const char* name_img = "mark.jpeg";


	if (IMAGEM)
	{
		//////////// COM IMAGEM ///////////
		

		/*LandingMark mark;

		Mat img = imread(name_img);

		if (img.empty())
		{
			cout << "Imagem vazia!";
			return 0;
		}

		mark.setImage(img);

		img = mark.processImage();

		imshow("", img);

		waitKey();*/
	}
	else if(VIDEO)
	{
		/////////////// COM VIDEO //////////////


		LandingMark mark;

		VideoCapture cap(name_video);

		if(!cap.isOpened()){
			cout << "Error opening video stream or file" << endl;
			return -1;
		}

		Mat frame;

		cap >> frame;

		while (true)
		{
			cap >> frame;

			mark.setImage(frame);

			if (mark.drawEllipse())
			{
				cout << "X: " << mark.majorEllipse.center.x - mark.centerX << "  Y: " << mark.majorEllipse.center.y - mark.centerY << endl;
			}

			imshow("Frame", mark.imagem);

			waitKey(20);
		}
	}
}
