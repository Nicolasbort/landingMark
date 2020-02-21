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


size_t countoursSize;
Mat pointsf;
RotatedRect box;


class LandingMark
{
public:

	Mat imagem, kernel;

	int rows, cols;
	int centerX, centerY;
	int majorEllipseWidth, majorEllipseHeight;

	bool success;

	RotatedRect majorEllipse;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	size_t countoursSize;
	Mat pointsf;
	RotatedRect box;


	LandingMark()
	{
		this->kernel = Mat::ones(KERNEL_RESOLUTION, KERNEL_RESOLUTION, CV_8U);

		this->majorEllipseWidth = 0;
		this->majorEllipseHeight = 0;
	
		this->success = false;
	}


	void CamParam(Mat img)
	{
		this->rows = img.rows;
		this->cols = img.cols;

		this->centerX = img.size().width/2;
		this->centerY = img.size().height/2;
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


	bool findEllipse()
	{
		this->processImage();

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

		return success;
	}


	void drawEllipse()
	{
		ellipse(this->imagem, this->majorEllipse.center, this->majorEllipse.size*0.5f, this->majorEllipse.angle, 0, 360, Scalar(255, 0, 0), 2);
	}



	Mat imlimiares(Mat hsv, int hsvMin[3], int hsvMax[3])
	{
		Mat hsvtresh;

		inRange(hsv, Scalar(hsvMin[0], hsvMin[1], hsvMin[2]), Scalar(hsvMax[0], hsvMax[1], hsvMax[2]), hsvtresh);

		return hsvtresh;
	}

	
	void printDistance()
	{
		cout << "X: " << this->majorEllipse.center.x - this->centerX << "  Y: " << this->majorEllipse.center.y - this->centerY << endl;
	}


	void show()
	{
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
		

		LandingMark mark;

		Mat img = imread(name_img);

		if (img.empty())
		{
			cout << "Imagem vazia!";
			return 0;
		}

		mark.setImage(img);

		if ( mark.findEllipse() )
		{
			mark.drawEllipse();
			mark.printDistance();
		}

		mark.show();

		waitKey();
	}
	else if(VIDEO)
	{
		/////////////// COM VIDEO //////////////


		LandingMark mark;

		VideoCapture cap(name_video);

		if(!cap.isOpened()){
			cout << "Erro ao abrir o video" << endl;
			return -1;
		}

		Mat frame;

		cap >> frame;

		while (true)
		{
			cap >> frame;

			mark.setImage(frame);

			if ( mark.findEllipse() )
			{
				mark.drawEllipse();
				mark.printDistance();
			}

			mark.show();

			waitKey(20);
		}
	}
}
