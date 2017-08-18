#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BLACK 50
#define WHITE 220

//struct defining a single pixel of an image
typedef struct {
	char blue;
	char green;
	char red;
	int checked;
} pixel;

//struct defining a marker
typedef struct {
	int ID;
	int x, y;
	int centerAdress;
	int cornerAdress[4];
	double direction;

	int sample[9];
} marker;

int getWidth(char* header) {
	return (unsigned char)header[18] + ((unsigned char)header[19] << 8) + ((unsigned char)header[20] << 16) + ((unsigned char)header[21] << 24);
}

int getHeight(char* header) {
	return (unsigned char)header[22] + ((unsigned char)header[23] << 8) + ((unsigned char)header[24] << 16) + ((unsigned char)header[25] << 24);
}

void getImage(FILE* fp, pixel* imageData, int width, int height) {
	int i = 0;

	for(i = 0; i < width*height; i++) {
		//each pixel is stored in an struct as red green and blue values
		imageData[i].blue = getc(fp);
		imageData[i].green = getc(fp);
		imageData[i].red = getc(fp);
		imageData[i].checked = 0;
	}

}

void drawImage(pixel* imageData, int width, int height, char* header) {
	FILE* fp = fopen("output.bmp", "w");

	int i = 0;

	for(i = 0; i < 54; i++) {
		putc(header[i], fp);
	}

	for(i = 0; i < width*height; i++) {
		putc(imageData[i].blue, fp);
		putc(imageData[i].green, fp);
		putc(imageData[i].red, fp);
	}

	fclose(fp);
}

int isWhite(pixel x) {
	if((unsigned int)x.red > WHITE && (unsigned int)x.blue > WHITE && (unsigned int)x.green > WHITE) {
		return 1;
	}

	return 0;
}

int isBlack(pixel x) {
	if((unsigned int)x.red < BLACK && (unsigned int)x.blue < BLACK && (unsigned int)x.green < BLACK) {
		return 1;
	}

	return 0;
}

void colorPixel(pixel* x, char blue, char green, char red) {
	x->red = red;
	x->green = green;
	x->blue = blue;
}

int isBlackCorner(pixel* image, int i, int width, int height) {
	//to see if a pixel is a corner, if less than 4 of the adjacent 8 pixels are black, it is a corner
	int counter = 0;

	if(isBlack(image[i])) {
		if(isBlack(image[i + 1])) {
			counter++;
		}
		if(isBlack(image[i - 1])) {
			counter++;
		}
		if(isBlack(image[i + width])) {
			counter++;
		}
		if(isBlack(image[i - width])) {
			counter++;
		}
		if(isBlack(image[i + 1 - width])) {
			counter++;
		}
		if(isBlack(image[i + 1 + width])) {
			counter++;
		}
		if(isBlack(image[i - 1 - width])) {
			counter++;
		}
		if(isBlack(image[i - 1 + width])) {
			counter++;
		}

		if(counter < 4) {
			return 1;
		}
	}

	return 0;
}

int getIndex(int val, int* corners, int numberOfCorners) {
	int i = 0;

	for(i = 0; i < numberOfCorners; i++) {
		if(val == corners[i]) {
			return i;
		}
	}

	return -1;
}

void isTouching(pixel* imageData, int i, int width, int* corners, int numberOfCorners, int* touchingCorners) {

	imageData[i].checked = 1;
	int tempIndex = 0;
	int cornerIndex = getIndex(i, corners, numberOfCorners);

	if(cornerIndex != -1) {
		touchingCorners[cornerIndex] = corners[cornerIndex];
	}

	if(!isBlack(imageData[i - 1]) || imageData[i - 1].checked) {
		if(!isBlack(imageData[i + 1]) || imageData[i + 1].checked) {
			if(!isBlack(imageData[i - width]) || imageData[i - width].checked) {
				if(!isBlack(imageData[i + width]) || imageData[i + width].checked) {
					return;
				}
			}
		}
	}

	if(isBlack(imageData[i - 1]) && !imageData[i - 1].checked) {
		isTouching(imageData, i - 1, width, corners, numberOfCorners, touchingCorners);
	}
	if(isBlack(imageData[i + 1]) && !imageData[i + 1].checked) {
		isTouching(imageData, i + 1, width, corners, numberOfCorners, touchingCorners);
	}
	if(isBlack(imageData[i - width]) && !imageData[i - width].checked) {
		isTouching(imageData, i - width, width, corners, numberOfCorners, touchingCorners);
	}
	if(isBlack(imageData[i + width]) && !imageData[i + width].checked) {
		isTouching(imageData, i + width, width, corners, numberOfCorners, touchingCorners);
	}

	return;
}

int numberOfNonZeroIndeces(int* array, int size) {
	int i = 0;
	int numberOfIndeces = 0;
	for(i = 0; i < size; i++) {
		if(array[i] != 0) {
			numberOfIndeces++;
		}
	}

	return numberOfIndeces;
}

int getNonZeroElement(int* array, int size, int index) {
	int i = 0;
	for(i = 0; i < size; i++) {
		if(array[i] != 0) {

			if(index == 0) {
				return array[i];
			}

			index--;
		}
	}

	return -1;

}

void printAllMarkers(marker* markers, int numberOfMarkers) {
	int i = 0;
	for(i = 0; i < numberOfMarkers; i++) {
		printf("Marker #%d: \n", markers[i].ID);
		printf("  direction: %f\n", markers[i].direction);
		printf("  center adress: %d\n", markers[i].centerAdress);
		printf("  x: %d, y: %d\n", markers[i].x, markers[i].y);
	}
}

void generateSample(marker* target, int width) {
	//this function has to return the 9 indexes in the image that correspond to the sample points
	int tempPoint = target->cornerAdress[1];
	int dx, dy = 0;
	int i;
	int indexes[9] = {8, 5, 2, 7, 4, 1, 6, 3, 0};
	int distance[3] = {3, 5, 7};
	int switcher = 0;

	//now we want to grab the sample points:
	int anchorPointOne[3];
	int anchorPointTwo[3];

	dx = target->cornerAdress[1] % width - target->cornerAdress[0] % width;
	tempPoint = target->cornerAdress[1];
	dy = 0;

	while(abs(tempPoint - target->cornerAdress[0]) > width) {
		tempPoint -= width;
		dy++;
	}

	for(i = 0; i < 3; i++) {
		anchorPointOne[i] = target->cornerAdress[0] + distance[i]*dx/10;
		anchorPointOne[i] += width*(dy*distance[i]/10);
	}

	

	dx = target->cornerAdress[3] % width - target->cornerAdress[2] % width;
	tempPoint = target->cornerAdress[3];
	dy = 0;

	while(abs(tempPoint - target->cornerAdress[2]) > width) {
		tempPoint -= width;
		dy++;
	}

	for(i = 0; i < 3; i++) {
		anchorPointTwo[i] = target->cornerAdress[2] + distance[i]*dx/10;
		anchorPointTwo[i] += width*(dy*distance[i]/10);
	}
	//these cases adress when the marker is at an odd perspective
	if(target->cornerAdress[3] % width - target->cornerAdress[2] % width > 0) {
		if(target->cornerAdress[1] % width - target->cornerAdress[0] % width < 0) {
			//switch order of anchorPointTwo[]
			switcher = anchorPointTwo[0];
			anchorPointTwo[0] = anchorPointTwo[2];
			anchorPointTwo[2] = switcher;
		}
	}

	if(target->cornerAdress[3] % width - target->cornerAdress[2] % width < 0) {
		if(target->cornerAdress[1] % width - target->cornerAdress[0] % width > 0) {
			//switch order of anchorPointTwo[]
			switcher = anchorPointTwo[0];
			anchorPointTwo[0] = anchorPointTwo[2];
			anchorPointTwo[2] = switcher;
		}
	}

	//these cases will rectify the anchor points such that the points read in a proper array format
	if(target->cornerAdress[0] % width - target->cornerAdress[1] % width < 0) {
		switcher = anchorPointTwo[0];
		anchorPointTwo[0] = anchorPointTwo[2];
		anchorPointTwo[2] = switcher;

		switcher = anchorPointOne[0];
		anchorPointOne[0] = anchorPointOne[2];
		anchorPointOne[2] = switcher;
	}

	//now that we have 2 anchor points we have to estimate the locations of the sample points
	//dy and dx now refer to the difference in x and y in the anchor points:

	int dxAnchor[3];
	int dyAnchor[3];

	for(i = 0; i < 3; i++) {
		dyAnchor[i] = 0;
		dxAnchor[i] = anchorPointOne[i] % width - anchorPointTwo[i] % width;
		tempPoint = anchorPointTwo[i];

		while(abs(tempPoint - anchorPointOne[i]) > width) {
			tempPoint -= width;
			dyAnchor[i]++;
		}	
	}

	
	for(i = 0; i < 9; i++) {
		target->sample[indexes[i]] = anchorPointOne[i / 3];
		target->sample[indexes[i]] += width * (dyAnchor[i / 3]*distance[i % 3]/10);
		target->sample[indexes[i]] -= dxAnchor[i / 3]*distance[i % 3]/10;
	}

	target->centerAdress = target->sample[4];
}

void getDirection(pixel* imageData, marker* target, int width) {
	int i = 0;
	int numOfBlackPixels = 0;
	int dirPixels[4];
	int pixelSelect;
	int dx, dy = 0;
	int tempPoint;

	for(i = 0; i < 9; i++) {
		if(i % 2 == 1) {
			dirPixels[i / 2] = target->sample[i];
		}
		target->sample[i] = isBlack(imageData[target->sample[i]]);
	}

	//direction pixels are sample[1, 3, 5, 7]
	//if more than one of these pixels are black, then it is not a marker. if none of these
	//pixels are black, then it is also not a marker. in these cases we will set direction to -1.

	for(i = 1; i < 9; i += 2) {
		if(target->sample[i]) {
			pixelSelect = i / 2;
		}
		numOfBlackPixels += target->sample[i];
	}

	if(numOfBlackPixels != 1) {
		target->direction = -1;
		return;
	}

	//now we have to calculate the direction of the marker
	//the pixel that is black identifies which direction is forward, but we still need to calculate the direction. to do this we will need
	//the location of the center and the direction pixel of the marker. dirPixels[pixelSelect] is the location of the direction pixel.
	//direction will be specified from 0 - 360 in terms of degrees with 0 being right and 180 being left.

	dx = dirPixels[pixelSelect] % width - target->centerAdress % width;
	if(target->centerAdress > dirPixels[pixelSelect]) {
		//dy should be negative in this case
		tempPoint = target->centerAdress;

		while(abs(tempPoint - dirPixels[pixelSelect]) > width) {
			tempPoint -= width;
			dy++;
		}

		dy *= -1;
	} else {
		tempPoint = dirPixels[pixelSelect];

		while(abs(tempPoint - target->centerAdress) > width) {
			tempPoint -= width;
			dy++;
		}
	}

	if(dx == 0) {
		if(dy > 0) {
			target->direction = (double)90;
		} else {
			target->direction = (double)(270);
		}

		return;
	}

	target->direction = atan(((double)dy)/((double)dx)) * 180 / 3.1415179;

	if(dx < 0) {
		target->direction += 180;
	} else {
		if(dy < 0) {
			target->direction += 360;
		}
	}
}

void getID(pixel* imageData, marker* target) {
	if(target->direction != -1) {
		//first we need to find the direction pixel i because that gives reference for the rest of the pixels
		int i;
		int dirIndex;
		int id;

		int indexSet[4][5] = {{0, 2, 4, 6, 8}, {6, 0, 4, 8, 2}, {2, 8, 4, 0, 6}, {8, 6, 4, 2, 0}};

		for(i = 1; i < 9; i += 2) {
			if(target->sample[i]) {
				dirIndex = i / 2;
			}
		}

		for(i = 0; i < 5; i++) {
			id += pow(2, i) * target->sample[indexSet[dirIndex][i]];
		}

		target->ID = id;
	} else {
		target->ID = -1;
	}

	return;
}

void drawLine(pixel* imageData, int width, int b, int g, int r, int pointOne, int pointTwo) {
	//first thing we need to do is find the slope which is dy/dx
	int dx, dy = 0;
	int editPoint, startPoint, i, tempPoint, endPoint;

	dx = -(pointOne % width - pointTwo % width);

	if(pointOne > pointTwo) {
		editPoint = pointOne;
		startPoint = pointTwo;
	} else {
		editPoint = pointTwo;
		startPoint = pointOne;
	}

	endPoint = editPoint;

	while(abs(editPoint - startPoint) > width) {
		editPoint -= width;
		dy++;
	}

	if(dx == 0) {
		tempPoint = startPoint;
		for(i = 0; i < dy; i++) {
			colorPixel(&imageData[tempPoint], b, g, r);
			tempPoint += width;
		}
		colorPixel(&imageData[tempPoint], b, g, r);
		colorPixel(&imageData[tempPoint + width], b, g, r);

		return;
	}

	if(dy == 0) {
		tempPoint = startPoint;
		if(dx > 0) {
			for(i = 0; i < abs(dx); i++) {
				colorPixel(&imageData[tempPoint], b, g, r);
				tempPoint++;
			}
		} else {
			for(i = 0; i < abs(dx); i++) {
				colorPixel(&imageData[tempPoint], b, g, r);
				tempPoint--;
			}
		}

		return;
	}

	float slope = (float)(abs(dy)) / (float)(abs(dx));
	float inverseSlope = (float)(abs(dx)) / (float)(abs(dy));
	float error = 0;

	if(abs(dy) > abs(dx)) {
		tempPoint = startPoint;
		while(endPoint > tempPoint) {
			for(i = 0; i < (int)slope; i++) {
				colorPixel(&imageData[tempPoint], b, g, r);
				tempPoint += width;
			}
			error += slope - (int)slope;
			if(error > 1) {
				colorPixel(&imageData[tempPoint], b, g, r);
				tempPoint += width;
				colorPixel(&imageData[tempPoint], b, g, r);
				error--;
			}

			if(dx > 0) {
				tempPoint++;
			} else {
				tempPoint--;
			}
		}
		colorPixel(&imageData[endPoint], b, g, r);
	} else if(abs(dx) > abs(dy)) {
		tempPoint = startPoint + width;
		while(endPoint > tempPoint) {
			for(i = 0; i < (int)inverseSlope; i++) {
				colorPixel(&imageData[tempPoint], b, g, r);
				if(dx > 0) {
					tempPoint++;
				} else {
					tempPoint--;
				}
			}
			error += inverseSlope - (int)inverseSlope;
			if(error > 1) {
				colorPixel(&imageData[tempPoint], b, g, r);
				if(dx > 0) {
					tempPoint++;
				} else {
					tempPoint--;
				}
				colorPixel(&imageData[tempPoint], b, g, r);
				error--;
			}

			tempPoint += width;
		}
		colorPixel(&imageData[endPoint], b, g, r);
	}
}

marker* detectMarkers(pixel* imageData, char* header, int width, int height) {
	int i = 0, j = 0;
	int numberOfCorners = 0;
	int* corners = (int*)malloc(sizeof(int));

	for(i = 0; i < width*height; i++) {

		//this locates all of the black corners in the image and stores them in the corners array
		if(isBlackCorner(imageData, i, width, height)) {
			numberOfCorners++;
			corners = (int*)realloc(corners, numberOfCorners*sizeof(int));
			corners[numberOfCorners - 1] = i;
		}
	}

	int touchingCorners[numberOfCorners];
	for(i = 0; i < numberOfCorners; i++) {
		touchingCorners[i] = 0;
	}

	i = 0;
	int cornersLeft = numberOfCorners;
	int numberOfMarkers = 0;
	marker* potentialMarkers = (marker*)malloc(sizeof(marker));

	//now we have to figure out which corners are touching. when there are 4 corners, we can make a marker
	while(cornersLeft) {
		//isTouching() is a recursive function that checks each of the pixels adjacent to the input pixel that is black
		//to determine which corners belong to the same shape.
		isTouching(imageData, getNonZeroElement(corners, numberOfCorners, 0), width, corners, numberOfCorners, touchingCorners);
		for(j = 0; j < numberOfCorners; j++) {
			corners[j] -= touchingCorners[j];
		}
		cornersLeft = numberOfNonZeroIndeces(corners, numberOfCorners);

		if(numberOfNonZeroIndeces(touchingCorners, numberOfCorners) == 4) {
			numberOfMarkers++;
			potentialMarkers = (marker*)realloc(potentialMarkers, numberOfMarkers*sizeof(marker));
			potentialMarkers[numberOfMarkers - 1].cornerAdress[0] = getNonZeroElement(touchingCorners, numberOfCorners, 0);
			potentialMarkers[numberOfMarkers - 1].cornerAdress[1] = getNonZeroElement(touchingCorners, numberOfCorners, 1);
			potentialMarkers[numberOfMarkers - 1].cornerAdress[2] = getNonZeroElement(touchingCorners, numberOfCorners, 2);
			potentialMarkers[numberOfMarkers - 1].cornerAdress[3] = getNonZeroElement(touchingCorners, numberOfCorners, 3);
		}

		for(j = 0; j < numberOfCorners; j++) {
			touchingCorners[j] = 0;
		}
	}

	//now we have a set of potential markers which are each black quadrilaterals. To further check the markers, we will have to check the
	//internal structure of the markers. We will begin by first deciding what the marker should look like. Below is an example

	for(i = 0; i < numberOfMarkers; i++) {
		generateSample(&(potentialMarkers[i]), width);

		//now we need to read the markers to determine if (1) it is actually a marker (2) the state of the marker
		getDirection(imageData, &(potentialMarkers[i]), width);
		getID(imageData, &(potentialMarkers[i]));
	}

	//now we can truncate to shrink the list of markers to only the ones with ID != -1
	int count = 0;
	for(i = 0; i < numberOfMarkers; i++) {
		if(potentialMarkers[i].ID != -1) {
			count++;
		}
	}

	j = 0;
	marker* markers = (marker*)malloc(count * sizeof(marker));
	for(i = 0; i < numberOfMarkers; i++) {
		if(potentialMarkers[i].ID != -1) {
			markers[j].ID = potentialMarkers[i].ID;
			markers[j].centerAdress = potentialMarkers[i].centerAdress;
			markers[j].direction = potentialMarkers[i].direction;
			markers[j].cornerAdress[0] = potentialMarkers[i].cornerAdress[0];
			markers[j].cornerAdress[1] = potentialMarkers[i].cornerAdress[1];
			markers[j].cornerAdress[2] = potentialMarkers[i].cornerAdress[2];
			markers[j].cornerAdress[3] = potentialMarkers[i].cornerAdress[3];

			//now we want to take the center adress and make it an x and y coordinate:
			markers[j].x = markers[j].centerAdress % width;
			markers[j].y = markers[j].centerAdress / width;
			j++;
		}
	}

	printAllMarkers(markers, count);

	//now we want to make changes to the image (I.e. hilight the markers)
	for(i = 0; i < count; i++) {
		//first we need to check if the marker is in a weird perspective:
		if(markers[i].cornerAdress[3] % width - markers[i].cornerAdress[2] % width < 0) {
			if(markers[i].cornerAdress[1] % width - markers[i].cornerAdress[0] % width > 0) {
				//weird case:
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0], markers[i].cornerAdress[1]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0] - width, markers[i].cornerAdress[1] - width);

				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[2], markers[i].cornerAdress[3]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[2] + width, markers[i].cornerAdress[3] + width);

				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0], markers[i].cornerAdress[3]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0] - 1, markers[i].cornerAdress[3] - 1);

				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[1], markers[i].cornerAdress[2]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[1] + 1, markers[i].cornerAdress[2] + 1);

				continue;
			}
		}

		if(markers[i].cornerAdress[3] % width - markers[i].cornerAdress[2] % width > 0) {
			if(markers[i].cornerAdress[1] % width - markers[i].cornerAdress[0] % width < 0) {
				//weird case:
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0], markers[i].cornerAdress[1]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0] - width, markers[i].cornerAdress[1] - width);

				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[2], markers[i].cornerAdress[3]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[2] + width, markers[i].cornerAdress[3] + width);

				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0], markers[i].cornerAdress[3]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0] - 1, markers[i].cornerAdress[3] - 1);

				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[1], markers[i].cornerAdress[2]);
				drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[1] + 1, markers[i].cornerAdress[2] + 1);

				continue;
			}
		} 

		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0], markers[i].cornerAdress[1]);
		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0] - width, markers[i].cornerAdress[1] - width);

		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[2], markers[i].cornerAdress[3]);
		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[2] + width, markers[i].cornerAdress[3] + width);

		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0], markers[i].cornerAdress[2]);
		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[0] - 1, markers[i].cornerAdress[2] - 1);

		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[1], markers[i].cornerAdress[3]);
		drawLine(imageData, width, 0, 255, 0, markers[i].cornerAdress[1] + 1, markers[i].cornerAdress[3] + 1);
	}

	drawImage(imageData, width, height, header);
	return markers;
}

int main(int argc, char* argv[]) {

	int i = 0;
	int width, height;
	char header[54];

	if(argc == 1) {
		printf("Format should be <executable> <image file>\n");
		return 0;
	}

	FILE* image = fopen(argv[1], "r");

	for(i = 0; i < 54; i++) {
		header[i] = getc(image);
	}

	width = getWidth(header);
	height = getHeight(header);

	pixel imageData[width*height];
	getImage(image, imageData, width, height);

	marker* markers = detectMarkers(imageData, header, width, height);

	fclose(image);
}