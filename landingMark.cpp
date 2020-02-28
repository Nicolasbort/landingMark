#include<opencv2/opencv.hpp>
#include<iostream>

using namespace cv;
using namespace std;


//limiares de saturacao e valor
#define MINSAT 50
#define MAXSAT 255

#define MINVAL 50
#define MAXVAL 255

// Limiares da cor amarela
#define MINYELLOW 10
#define MAXYELLOW 40

// Limiares da cor azul
#define MINBLUE 100
#define MAXBLUE 140


int ARR_MAXYELLOW[3] = {MAXYELLOW, MAXSAT, MAXVAL};
int ARR_MINYELLOW[3] = {MINYELLOW, MINSAT, MINVAL};

int ARR_MAXBLUE[3] = {MAXBLUE, MAXSAT, MAXVAL};
int ARR_MINBLUE[3] = {MINBLUE, MINSAT, MINVAL};

//parametros de filtros
int GAUSSIAN_FILTER = 5;
int KERNEL_RESOLUTION = 7;

//dimensoes da base real
float ARESTA = 500.0f; //aresta da base (em mm)
float RAIO = 200.0f; //raio do centro da base 
int RESOLUTION = 50;

size_t countoursSize;
Mat pointsf;
RotatedRect box;

Mat blue_rect, yellow_circle;


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
		Mat bitwise_img;

        cvtColor(this->imagem, bitwise_img, COLOR_BGR2HSV);

		blue_rect = this->imlimiares(bitwise_img, ARR_MINBLUE, ARR_MAXBLUE);
		yellow_circle = this->imlimiares(bitwise_img, ARR_MINYELLOW, ARR_MAXYELLOW);

		morphologyEx(blue_rect, blue_rect, MORPH_CLOSE, this->kernel, Point(-1,-1), 2);

		findContours(blue_rect, this->contours, this->hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0,0));

		this->imagem = blue_rect;
	}


	bool findEllipse()
	{
		this->processImage();

		bool success = false;

		// Remove alguns falsos positivos
		if (this->contours.size() <= 30)
		{
			for(size_t i = 0; i < this->contours.size(); i++)
			{
				countoursSize = this->contours[i].size();

				if (countoursSize < 5)
					continue;

				Mat(this->contours[i]).convertTo(pointsf, CV_32F);
				box = fitEllipse(pointsf);
				
				// Elimina elipses pequenas
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



int main(int argc, char* argv[])
{

	if (argc < 2)
	{
		cerr << "Rodar o código > ./NomeDoArquivo \"NomeDoVideo\"\n";
		return -1;
	}

	char* file_name = argv[1];

	bool VIDEO = true;

	if (VIDEO)
	{

		LandingMark mark;

		VideoCapture cap(file_name);

		if (!cap.isOpened())
		{
			cout << "Erro ao abrir o video" << endl;
			return -1;
		}

		Mat frame;

		cap >> frame;

		while (true)
		{
			cap >> frame;

			if (frame.empty())
				break;

			mark.setImage(frame);

			if ( mark.findEllipse() )
			{
				mark.drawEllipse();
				mark.printDistance();
			}

			mark.show();

			waitKey(10);
		}
		
	}
	else
	{
		
		LandingMark mark;

		Mat img = imread(file_name);

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
}
