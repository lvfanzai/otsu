#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "EasyBMP/EasyBMP.h"


/*
阀值是图像二值化处理中非常重要的一个参数，意思就是：如果灰度图像的像素
颜色值大于该阀值就把该点当作黑色，小于该阀值就把改点当作白色。最简单的
做法就是把阀值取为127（255的一半），但是这种做法是不科学的，处理后的二
值化图像效果也很不理想。关于阀值的选取是一门很深的学问，有很多经典的
算法用于选取该阀值，理论我就不做过多描述了，感兴趣的自己趣网络上搜索
相关资料，我这里采用的是比较经典的大律法(OTSU)。算法代码来自网络。 
*/ 
int findThreshold(BMP frame) //大津法求阈值
{
#define GrayScale 256	//frame灰度级
	int width = frame.TellWidth(); 
	int height = frame.TellHeight();
	int pixelCount[GrayScale]={0};
	float pixelPro[GrayScale]={0};
	int i, j, pixelSum = width * height, threshold = 0;
	int r=0 ,g=0 ,b=0 ,data = 0;

	//统计每个灰度级中像素的个数
	for(i = 0; i < height; i++)
	{
		for(j = 0;j < width;j++)
		{
			r = frame(j,i)->Red;
			g = frame(j,i)->Green;
			b = frame(j,i)->Blue;
			
			data = pow((pow(r, 2.2) * 0.2973 + 
			pow(g, 2.2)* 0.6274 + 
			pow(b, 2.2) * 0.0753),(1 / 2.2));
			
			pixelCount[data]++;
		}
	}

	//计算每个灰度级的像素数目占整幅图像的比例
	for(i = 0; i < GrayScale; i++)
	{
		pixelPro[i] = (float)pixelCount[i] / pixelSum;
	}

	//遍历灰度级[0,255],寻找合适的threshold
	float w0, w1, u0tmp, u1tmp, u0, u1, deltaTmp, deltaMax = 0;
	for(i = 0; i < GrayScale; i++)
	{
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = deltaTmp = 0;
		for(j = 0; j < GrayScale; j++)
		{
			if(j <= i)   //背景部分
			{
				w0 += pixelPro[j];
				u0tmp += j * pixelPro[j];
			}
			else   //前景部分
			{
				w1 += pixelPro[j];
				u1tmp += j * pixelPro[j];
			}
		}
		u0 = u0tmp / w0;
		u1 = u1tmp / w1;
		deltaTmp = (float)(w0 *w1* pow((u0 - u1), 2)) ;
		if(deltaTmp > deltaMax)
		{
			deltaMax = deltaTmp;
			threshold = i;
		}
	}
	return threshold;
}

int main(int argc, char *argv[])
{
	int nFrames;
	BMP Input;
	FILE *fp;
	unsigned int n = 0;
	//fp = fopen("output.bin", "wb"); /* 输出文件 */
	fp = fopen("output.h", "wb"); /* 输出文件 */
	printf("******************************************************************\n");
	printf("*                               说  明                           *\n");
	printf("*                                                                *\n");
	printf("*      本软件用于把24位色的BMP图像帧转换成用于LCD显示的二进制数  *\n");
	printf("*  据文件。图片名必须为从00000开始的连续数字，总共5位，不足五位  *\n");
	printf("*  要在前面用0补齐（也就是说本软件最多能处理99999张图片！)。如： *\n");
	printf("*  00008.bmp。请把本软件和图片文件放在同一目录下！               *\n");
	printf("******************************************************************\n");
	printf(">请输入图片数目: ");
	scanf("%d", &nFrames);
	
	printf("\n>转换开始......\n");
	char FullPath[255], FileName[20];
	
	for (int Frame=0; Frame<nFrames; Frame++)
	{
		strcpy(FileName, "00000.bmp");
		
		FileName[4] += (Frame/1) % 10;
		FileName[3] += (Frame/10) % 10;
		FileName[2] += (Frame/100) % 10;
		FileName[1] += (Frame/1000) % 10;
		FileName[0] += (Frame/10000) % 10;
		/*if(Frame > 9999)
		{
			strcpy(FileName, "000000.bmp");
			FileName[5] += (Frame/1) % 10;
			FileName[4] += (Frame/10) % 10;
			FileName[3] += (Frame/100) % 10;
			FileName[2] += (Frame/1000) % 10;
			FileName[1] += (Frame/10000) % 10;
			FileName[0] += (Frame/100000) % 10;
		}*/
		
		fputs(FileName, fp);
		fputs("[ ] = \r\n{\r\n", fp);
		
		printf("\r>正在处理: %s", FileName);
		
		strcpy(FullPath, ""); 
		strcat(FullPath, FileName);
		if(Input.ReadFromFile(FullPath) == false)
		{
			printf("\n>打开图片文件出错！\n");
			fclose(fp);
			fp = NULL;
			return -1;
		} 
		
		int nSeg;
		
		int threshold = findThreshold(Input);//阀值 

		nSeg = Input.TellHeight() / 8;
		for (int iSeg=0; iSeg<nSeg; iSeg++)
		{
			for (int x=0; x<Input.TellWidth(); x++)
			{
				unsigned char Data;
				char Outdat[5] = {0};
				Data = 0x00;
				for (int j=0; j<8; j++)
				{
					int y = iSeg*8 + j;
					int r = Input(x,y)->Red;
					int g = Input(x,y)->Green;
					int b = Input(x,y)->Blue;
					
					/*int brightness = (int)floor(
						0.299*Input(x,y)->Red + 
						0.587*Input(x,y)->Green + 
						0.114*Input(x,y)->Blue);*/
					int brightness = pow((pow(r, 2.2) * 0.2973 + 
						pow(g, 2.2)* 0.6274 + 
						pow(b, 2.2) * 0.0753),(1 / 2.2));
					
					if (brightness > 255) brightness = 255;
					if (brightness < 0) brightness = 0;
					if (brightness > threshold) Data |= (0x01 << j);
				}
				sprintf(Outdat,"0x%02X,",Data); 
				fputs(Outdat, fp);
				n++;
				if(n >= 16)
				{
					n = 0;
					fputs("\r\n", fp);
				}
			}
		}
		fputs("\r\n};\r\n", fp);
	}
	printf("\n>转换结束......\n");
	fclose(fp);
	fp = NULL;
	printf("\n>按任意键退出！\n");
	getchar(); 
	getchar(); 
	return 0;
}
 
