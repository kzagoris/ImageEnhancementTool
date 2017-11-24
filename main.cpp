#include <stdio.h>
#include <string>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

#include "niblack.h"
#include "anisotropicdiffusion.h"
#include "offcell.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
	printf("\nUsage: enhancement [infile] [outfile]\n");
	return -1;
    }
    string NameIn = argv[1];
    string NameOut = argv[2];
    //Read Image
    Mat ImageIn;
    ImageIn.data = NULL;
    ImageIn = imread(NameIn, CV_LOAD_IMAGE_GRAYSCALE);
	Mat Anisotropic = AnisotropicDiffusion(ImageIn, 10, 10);
    Mat OffCell = OffCellImageDynamic(Anisotropic,3 , 1);
    imwrite(NameOut, OffCell);
}
