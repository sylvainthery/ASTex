/*******************************************************************************
* ASTex:                                                                       *
* Copyright (C) IGG Group, ICube, University of Strasbourg, France             *
*                                                                              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: https://astex-icube.github.io                                      *
* Contact information: astex@icube.unistra.fr                                  *
*                                                                              *
*******************************************************************************/



#include <iostream>
#include "itkImageToImageFilter.h"
#include "itkInPlaceImageFilter.h"


#include <ASTex/image_gray.h>
#include <ASTex/image_rgb.h>


using namespace ASTex;


/**
 * Example of filter that use ASTex-syntax
 *   RGBu8 -> Grayu8
 */
class SimpleFilter: public itk::ImageToImageFilter< ImageRGBu8::ItkImg, ImageGrayu8::ItkImg >
{
public:
	// Standard class typedefs & macros for ikt
	typedef SimpleFilter                                                       Self;
	typedef itk::ImageToImageFilter< ImageRGBu8::ItkImg, ImageGrayu8::ItkImg > Superclass;
	typedef itk::SmartPointer< Self >                                          Pointer;
	typedef itk::SmartPointer< const Self >                                    ConstPointer;
	itkNewMacro(Self);
	itkTypeMacro(SimpleFilter, ImageToImageFilter);


	/// set center filter parameter
	inline void setCenter(const itk::Offset<2>& c)
	{
		center_ = c;
	}

	/// set radius filter parameter
	inline void setRadius(long r)
	{
		radius_ = r;
	}

protected:

	// filter params
	Offset center_;
	long radius_;

	// protected constructor (forbid usage of new and variable declaration)
	SimpleFilter():
		center_({{0,0}}),
		radius_(1)
	{}

	virtual ~SimpleFilter() {}

	//
	// overriden method that generate the output
	//
	void GenerateData() ITK_OVERRIDE
	{
		//  we are responsible of allocating the images buffers
		// in this simple case (output of same size than inputs) just call:
		this->AllocateOutputs();

		// create local images for ASTex syntax (no data copy, just a pointer)
		// note here pb of const ptr for input
		ConstImageRGBu8 img_in(this->GetInput());
		ImageGrayu8 img_out(this->GetOutput());

		// apply algo on region
		img_in.for_all_pixels([&img_out,this] (const ImageRGBu8::PixelType& p, int x, int y)
		{
			// very simple gray computation
			double d = p[0];
			d += p[1];
			d += p[2];
			d/= 3.0;
			// fading
			int nx = x - center_[0];
			int ny = y - center_[1];
			double fad = 1.0 - std::sqrt(nx*nx + ny*ny)/radius_;
			//set output pixel
			img_out.pixelAbsolute(x,y) = d*fad;
		});
	}

private:
	// to avoid filter object copy
	SimpleFilter(const Self &);
	void operator=(const Self &);
};



/**
 * Example of simple inplace filter (input == output)
 *  Grayu8 -> Grayu8 !
 */
class SimpleInPlaceFilter: public itk::InPlaceImageFilter< ImageGrayu8::ItkImg >
{
public:
	// Standard class typedefs & macros for ikt
	typedef SimpleInPlaceFilter                                                Self;
	typedef itk::ImageToImageFilter< ImageRGBu8::ItkImg, ImageGrayu8::ItkImg > Superclass;
	typedef itk::SmartPointer< Self >                                          Pointer;
	typedef itk::SmartPointer< const Self >                                    ConstPointer;
	itkNewMacro(Self);
	itkTypeMacro(SimpleInPlaceFilter, InPlaceImageFilter);


protected:

	// protected constructor (forbid usage of new and variable declaration)
	SimpleInPlaceFilter() {}

	virtual ~SimpleInPlaceFilter() {}

	//
	// overriden method that generate the output
	//
	void GenerateData() ITK_OVERRIDE
	{
		this->AllocateOutputs();

		// check validity of input/outoout
		if (!this->GetRunningInPlace())
		{
			std::cerr << "something is wrong!" << std::endl;
			return;
		}

		// create local images for ASTex syntax (no data copy, just a pointer)
		ImageGrayu8 img_out(this->GetOutput());
		// apply algo on region
		img_out.for_all_pixels([&] (ImageGrayu8::PixelType& p)
		{
			p = 255-p;
		});
	}

private:
	// to avoid filter object copy
	SimpleInPlaceFilter(const Self &);
	void operator=(const Self &);
};




/**
 * Example of multi-thread filter
 *    RGBu8 -> Grayu8
 */
class SimpleMTFilter: public itk::ImageToImageFilter< ImageRGBu8::ItkImg, ImageGrayu8::ItkImg >
{
public:
	// Standard class typedefs & macros for ikt
	typedef SimpleMTFilter                                                      Self;
	typedef itk::ImageToImageFilter< ImageRGBu8::ItkImg, ImageGrayu8::ItkImg > Superclass;
	typedef itk::SmartPointer< Self >                                          Pointer;
	typedef itk::SmartPointer< const Self >                                    ConstPointer;
	itkNewMacro(Self);
	itkTypeMacro(SimpleMTFilter, ImageToImageFilter);


	/// set center filter parameter
	inline void setCenter(const itk::Offset<2>& c)
	{
		center_ = c;
	}

	/// set radius filter parameter
	inline void setRadius(long r)
	{
		radius_ = r;
	}

protected:

	// filter params
	Offset center_;
	long radius_;

	// just for nice cout

	// protected constructor (forbid usage of new and variable declaration)
	SimpleMTFilter():
		center_({{0,0}}),
		radius_(1)
	{
		// ITK oll school MT
		this->DynamicMultiThreadingOff();
	}

	virtual ~SimpleMTFilter() {}


	//
	// function that generate image
	// itk automatically cut image into regions to allow multi-thread
	//
	void ThreadedGenerateData(const Region& region, itk::ThreadIdType threadId) ITK_OVERRIDE
	{
		// create local images for ASTex syntax (no data copy, just a pointer)
		ConstImageRGBu8 img_in(this->GetInput());
		ImageGrayu8 img_out(this->GetOutput());

		// apply algo on region
		img_in.for_region_pixels(region,[&] (const ImageRGBu8::PixelType& p, int x, int y)
		{
			// very simple gray computation
			double d = p[0];
			d += p[1];
			d += p[2];
			d/= 3.0;
			// fading
			int nx = x - center_[0];
			int ny = y - center_[1];
			double fad = 1.0 - std::sqrt(nx*nx + ny*ny)/radius_;
			//set output pixel
			img_out.pixelAbsolute(x,y) = d*fad;
		});
	}

private:
	// to avoid filter object copy
	SimpleMTFilter(const Self &);
	void operator=(const Self &);
};




int main()
{

	ImageRGBu8 image;

	bool ok = image.load(TEMPO_PATH+"simpleRGB.png");
	if (!ok)
		return 1;

	long w = image.width();
	long h = image.height();
	image.setCenter(w/2,h/2);


	SimpleFilter::Pointer filter1 = SimpleFilter::New();
	// set input of filter
	filter1->SetInput(image.itk());
	// add external info for filter
	filter1->setCenter(image.getCenter());
	// add external info
	filter1->setRadius(w/1.414);

	SimpleInPlaceFilter::Pointer filter2 = SimpleInPlaceFilter::New();
	// set input of filter2
	filter2->SetInput(filter1->GetOutput());

	// create image from output of filter
	ImageGrayu8 img_out(filter2->GetOutput());
	// launch filter
	filter2->Update();

	img_out.save(TEMPO_PATH+"tuto_filter2.png");


	SimpleMTFilter::Pointer filtermt = SimpleMTFilter::New();

	// set input of filter
	filtermt->SetInput(image.itk());
	// add external info for filter
	filtermt->setCenter(image.getCenter());
	// add external info
	filtermt->setRadius(w/1.414);

	// create image from output of filter
	ImageGrayu8 img_out2(filtermt->GetOutput());
	// launch filter
	filtermt->Update();
	img_out2.save(TEMPO_PATH+"tuto_filter2mt.png");



  return EXIT_SUCCESS;
}


