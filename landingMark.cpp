#include "../include/defines.hpp"

using namespace cv;
using namespace std;

#define DEGGUBCOLOR false

size_t countoursSize;
Mat pointsf;
RotatedRect box;


// Arrays utilizados no inRange
int ARR_MAXBLUE[3]   = {MAXBLUE, MAXSATBLUE, MAXVALBLUE};
int ARR_MINBLUE[3]   = {MINBLUE, MINSATBLUE, MINVALBLUE};

int ARR_MAXYELLOW[3] = {MAXYELLOW, MAXSATYELLOW, MAXVALYELLOW};
int ARR_MINYELLOW[3] = {MINYELLOW, MINSATYELLOW, MINVALYELLOW};


bool isSquare(Rect rectangle)
{
	if (rectangle.x >= rectangle.y)
	{
		if ( (rectangle.x - rectangle.y) <= 0.2 * rectangle.x )
			return true;
		else
			return false;
	}
	else
	{
		if ( (rectangle.y - rectangle.x) <= 0.2 * rectangle.y )
			return true;
		else
			return false;
	}
}


class LandingMark
{
public:

	Mat mainImagem_C3, imageHSV_C3, image_blue_C1, image_yellow_C1, image_final_C1; 
	
	Mat kernel;

	int rows, cols;
	int centerX, centerY;
	int majorEllipseWidth, majorEllipseHeight;
	int minEllipseWidth, minEllipseHeight;

	bool success, fstTime;

	RotatedRect majorEllipse;

	Rect mark;

	vector<vector<Point> > contours;

	LandingMark()
	{
		this->kernel = Mat::ones(KERNEL_RESOLUTION, KERNEL_RESOLUTION, CV_8U);

		this->majorEllipseWidth = 0;
		this->majorEllipseHeight = 0;
	
		this->success = false;
		this->fstTime = true;
	}


	void CamParam(Mat img)
	{
		this->rows = img.rows;
		this->cols = img.cols;

		this->centerX = img.size().width/2;
		this->centerY = img.size().height/2;

		this->minEllipseHeight = 0.05 * this->rows;
		this->minEllipseWidth = 0.05 * this->cols;
	}


	void setImage(Mat img)
	{
		GaussianBlur(img, img, Size(GAUSSIAN_FILTER, GAUSSIAN_FILTER), 0);

		if (this->fstTime)
		{
			this->CamParam(img);
			this->fstTime = false;
		}

		this->mainImagem_C3 = img;
	}


	Mat imfill(Mat img)
	{
		morphologyEx(img, img, MORPH_CLOSE, this->kernel, Point(-1, -1), 3);

		findContours(img, this->contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	    vector<vector<Point>> hull( this->contours.size() );

		for( size_t i = 0; i < this->contours.size(); i++ )
		{
		 	convexHull( this->contours[i], hull[i] );
		}

		if (hull.size() == 1)
		{
			drawContours(img, hull, 0, 255, -1);
		}
		else if (hull.size() > 1)
		{
			float biggestArea = 0;
			vector<Point> biggestContour;

			for ( size_t i = 0; i < hull.size(); i++ )
			{
				float area = contourArea(hull[i]);

				if (area > biggestArea)
				{
					biggestArea = area;
					biggestContour = hull[i];
				}
			}
			vector<vector<Point>> bigContours;
			bigContours.push_back(biggestContour);
			drawContours(img, bigContours, 0, 255, -1);
		}

		return img;
	}

	Mat imlimiares(Mat hsv, int hsvMin[3], int hsvMax[3])
	{
		Mat hsvtresh;

		inRange(hsv, Scalar(hsvMin[0], hsvMin[1], hsvMin[2]), Scalar(hsvMax[0], hsvMax[1], hsvMax[2]), hsvtresh);

		hsvtresh = this->imfill(hsvtresh);

		return hsvtresh;
	}


	void processImage()
	{
        cvtColor(this->mainImagem_C3, this->imageHSV_C3, COLOR_BGR2HSV);

		Mat hsv, output;

// Usado para testar parametros de cores diferentes sem utilizar o imfill
#if DEBBUGCOLOR
		inRange(imageHSV_C3, Scalar(ARR_MINBLUE[0], ARR_MINBLUE[1], ARR_MINBLUE[2]), Scalar(ARR_MAXBLUE[0], ARR_MAXBLUE[1], ARR_MAXBLUE[2]), image_blue_C1);
		inRange(imageHSV_C3, Scalar(ARR_MINYELLOW[0], ARR_MINYELLOW[1], ARR_MINYELLOW[2]), Scalar(ARR_MAXYELLOW[0], ARR_MAXYELLOW[1], ARR_MAXYELLOW[2]), image_yellow_C1);
#else	

		// Pega a area azul
		this->image_blue_C1 = this->imlimiares(this->imageHSV_C3, ARR_MINBLUE, ARR_MAXBLUE);
		bitwise_and(this->imageHSV_C3, this->imageHSV_C3, hsv, this->image_blue_C1);

		// Pega a area amarela
		this->image_yellow_C1 = this->imlimiares(this->imageHSV_C3, ARR_MINYELLOW, ARR_MAXYELLOW);
		bitwise_and(hsv, hsv, output, this->image_yellow_C1);
		
#endif

		// Pega apenas a area do mark
		bitwise_and(this->image_blue_C1, this->image_yellow_C1, this->image_final_C1);
	}


	bool findSquare()
	{
		this->processImage();

		findContours(this->image_final_C1, this->contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		bool success = false;

		for (int i=0; i<this->contours.size(); i++)
		{
			Rect currentRect = boundingRect( this->contours[i] );

			this->mark = currentRect;
			return true;

			/*
			if ( isSquare(currentRect) )
			{
				this->mark = currentRect;
				return true;
			}
			else
			{
				return false;
			}
			*/
		}
	}


	void drawSquare()
	{
		rectangle(this->mainImagem_C3, this->mark, Scalar(0, 255, 255), 2);
	}


	bool findEllipse()
	{
		this->processImage();

		findContours(this->image_final_C1, this->contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		bool success = false;


		for(size_t i = 0; i < this->contours.size(); i++)
		{
			countoursSize = this->contours[i].size();

			if (countoursSize < 5)
				continue;

			Mat(this->contours[i]).convertTo(pointsf, CV_32F);
			box = fitEllipse(pointsf);
			
			// Elimina elipses pequenas
			if ( box.size.width > this->minEllipseWidth && box.size.height > this->minEllipseHeight )
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
		ellipse(this->mainImagem_C3, this->majorEllipse.center, this->majorEllipse.size*0.5f, this->majorEllipse.angle, 0, 360, Scalar(255, 0, 0), 2);
	}


	
	void printDistance()
	{
		//cout << "X: " << this->majorEllipse.center.x - this->centerX << "  Y: " << this->majorEllipse.center.y - this->centerY << endl;
		cout << "( " << (mark.x + (mark.width/2)) - centerX << ", " << (mark.y + (mark.height/2)) - centerY << " )\n";
	}


	void show()
	{
		imshow("Main_image", this->mainImagem_C3);
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

		int countWrite = 0;

		while (true)
		{

			cap >> frame;

			//resize(frame, frame, Size(1200, 600));

			if (frame.empty())
				break;

			mark.setImage(frame);
			
			if ( mark.findSquare() )
			{
				mark.drawSquare();
				mark.printDistance();

				//circle(mark.mainImagem_C3, Point(mark.mark.x + (mark.mark.width/2), mark.mark.y + (mark.mark.height/2)), 30, 150, 3);
			}

			mark.show();

			int key = waitKey(20);

            // Pressionar espaço para salvar o frame atual
            if (key == 32)
			{
                imwrite("frame"+to_string(countWrite)+".jpeg", mark.mainImagem_C3);
				countWrite++;
			}
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
		mark.processImage();

		// if ( mark.findEllipse() )
		// {
		// 	mark.drawEllipse();
		// 	mark.printDistance();
		// }

		mark.show();

		waitKey();
	}
}
