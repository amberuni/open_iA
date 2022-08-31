/*************************************  open_iA  ************************************ *
* **********   A tool for visual analysis and processing of 3D CT images   ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2022  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan, Ar. &  Al. *
*                 Amirkhanov, J. Weissenböck, B. Fröhler, M. Schiwarth, P. Weinberger *
* *********************************************************************************** *
* This program is free software: you can redistribute it and/or modify it under the   *
* terms of the GNU General Public License as published by the Free Software           *
* Foundation, either version 3 of the License, or (at your option) any later version. *
*                                                                                     *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY     *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A     *
* PARTICULAR PURPOSE.  See the GNU General Public License for more details.           *
*                                                                                     *
* You should have received a copy of the GNU General Public License along with this   *
* program.  If not, see http://www.gnu.org/licenses/                                  *
* *********************************************************************************** *
* Contact: FH OÖ Forschungs & Entwicklungs GmbH, Campus Wels, CT-Gruppe,              *
*          Stelzhamerstraße 23, 4600 Wels / Austria, Email: c.heinzl@fh-wels.at       *
* ************************************************************************************/
#include "iAGeometricTransformations.h"

#include <defines.h>          // for DIM
#include <iADataSet.h>
#include <iAMathUtility.h>
#include <iAProgress.h>
#include <iAToolsITK.h>    // for setIndexOffsetToZero
#include <iAToolsVTK.h>    // for adjustIndexAndSizeToImage
#include <iATypedCallHelper.h>
#include <iAValueTypeVectorHelpers.h>

#include <itkBSplineInterpolateImageFunction.h>
#include <itkConstantPadImageFilter.h>
#include <itkImageIOBase.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkResampleImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkWindowedSincInterpolateImageFunction.h>

#include <vtkImageData.h>
#include <vtkImageExtractComponents.h>

#include <QtConcurrent/qtconcurrentfilter.h>

IAFILTER_CREATE(iAExtractComponent)

void iAExtractComponent::performWork(QVariantMap const& parameters)
{
	int const componentNr = parameters["Component to extract"].toInt();
	auto img = imageInput(0)->vtkImage();
	if (componentNr > img->GetNumberOfScalarComponents())
	{
		LOG(lvlWarn,
			QString("Invalid value for 'Component to extract': %1 exceeds valid range 1..%2")
				.arg(componentNr)
				.arg(img->GetNumberOfScalarComponents()));
		return;
	}
	auto extractFilter = vtkSmartPointer<vtkImageExtractComponents>::New();
	extractFilter->SetInputData(img);
	extractFilter->SetComponents(componentNr);
	progress()->observe(extractFilter);
	extractFilter->Update();
	addOutput(extractFilter->GetOutput());
}

iAExtractComponent::iAExtractComponent() :
	iAFilter("Extract Component", "Geometric Transformations",
		"Extract single component from multi-component image.<br/>"
		"For more information, see the "
		"<a href=\"https://vtk.org/doc/nightly/html/classvtkImageExtractComponents.html\">"
		"Extract Components</a> filter in the VTK documentation.")
{
	addParameter("Component to extract", iAValueType::Discrete, 1, 1, 1);
}

void iAExtractComponent::adaptParametersToInput(QVariantMap& /* params */, std::vector<std::shared_ptr<iADataSet>> const& dataSets)
{
	assert(dataSets.size() > 0 && dynamic_cast<iAImageData*>(dataSets[0].get()));
	paramsWritable()[0]->adjustMinMax(dynamic_cast<iAImageData*>(dataSets[0].get())->vtkImage()->GetNumberOfScalarComponents());
}


namespace
{
	const QString InterpLinear("Linear");
	const QString InterpNearestNeighbour("Nearest Neighbour");
	const QString InterpBSpline("BSpline");
	const QString InterpWindowedSinc("Windowed Sinc");
}

template<typename T> void simpleResampler(iAFilter* filter, QVariantMap const & parameters)
{
	double VoxelScale = 0.999; //Used because otherwise is a one voxel border with 0
	auto inImg = filter->imageInput(0)->itkImage();
	auto inSize = filter->imageInput(0)->vtkImage()->GetDimensions();

	typedef itk::Image<T, DIM> InputImageType;
	typedef itk::ResampleImageFilter<InputImageType, InputImageType> ResampleFilterType;
	auto resampler = ResampleFilterType::New();
	typename ResampleFilterType::SizeType size;
	setFromVectorVariant<int>(size, parameters["Size"]);
	typename ResampleFilterType::SpacingType spacing;
	spacing[0] = inImg->GetSpacing()[0] * (static_cast<double>(inSize[0]) / size[0] * VoxelScale);
	spacing[1] = inImg->GetSpacing()[1] * (static_cast<double>(inSize[1]) / size[1] * VoxelScale);
	spacing[2] = inImg->GetSpacing()[2] * (static_cast<double>(inSize[2]) / size[2] * VoxelScale);
	QString interpolatorName = parameters["Interpolator"].toString();
	if (interpolatorName == InterpLinear)
	{
		typedef itk::LinearInterpolateImageFunction<InputImageType, double> InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolatorName == InterpNearestNeighbour)
	{
		typedef itk::NearestNeighborInterpolateImageFunction<InputImageType, double> InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolatorName == InterpBSpline)
	{
		typedef itk::BSplineInterpolateImageFunction<InputImageType, double> InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolatorName == InterpWindowedSinc)
	{
		typedef itk::Function::HammingWindowFunction<3> WindowFunctionType;
		typedef itk::ZeroFluxNeumannBoundaryCondition<InputImageType> ConditionType;
		typedef itk::WindowedSincInterpolateImageFunction<
			InputImageType, 3,
			WindowFunctionType,
			ConditionType,
			double> InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	resampler->SetInput(dynamic_cast<InputImageType*>(inImg));
	resampler->SetOutputOrigin(inImg->GetOrigin());
	resampler->SetOutputSpacing( spacing );
	resampler->SetSize( size );
	resampler->SetDefaultPixelValue( 0 );
	filter->progress()->observe( resampler );
	resampler->Update( );
	filter->addOutput( resampler->GetOutput() );
}


IAFILTER_CREATE(iASimpleResampleFilter)

void iASimpleResampleFilter::performWork(QVariantMap const& parameters)
{
	ITK_TYPED_CALL(simpleResampler, inputPixelType(), this, parameters);
}

iASimpleResampleFilter::iASimpleResampleFilter() :
	iAFilter("Simple Resample", "Geometric Transformations",
		"Resample the image to a new size.<br/>"
		"For more information, see the "
		"<a href=\"https ://itk.org/Doxygen/html/classitk_1_1ResampleImageFilter.html\">"
		"Resample Filter</a> in the ITK documentation.")
{
	addParameter("Size", iAValueType::Vector3i, variantVector<int>({1, 1, 1}));
	QStringList interpolators;
	interpolators << InterpLinear << InterpNearestNeighbour << InterpBSpline << InterpWindowedSinc;
	addParameter("Interpolator", iAValueType::Categorical, interpolators);
}

void iASimpleResampleFilter::adaptParametersToInput(QVariantMap& params, std::vector<std::shared_ptr<iADataSet>> const& dataSets)
{
	assert(dataSets.size() > 0 && dynamic_cast<iAImageData*>(dataSets[0].get()));
	auto img = dynamic_cast<iAImageData*>(dataSets[0].get())->vtkImage();
	auto dim = img->GetDimensions();
	params["Size"] = variantVector<int>({dim[0], dim[1], dim[2]});
}



template <typename T>
void resampler(iAFilter* filter, QVariantMap const& parameters)
{
	typedef itk::Image<T, DIM> InputImageType;
	typedef itk::ResampleImageFilter<InputImageType, InputImageType> ResampleFilterType;
	auto resampler = ResampleFilterType::New();

	typename ResampleFilterType::OriginPointType origin;
	setFromVectorVariant<int>(origin, parameters["Origin"]);
	typename ResampleFilterType::SpacingType spacing;
	setFromVectorVariant<int>(spacing, parameters["Spacing"]);
	typename ResampleFilterType::SizeType size;
	setFromVectorVariant<int>(size, parameters["Size"]);
	QString interpolatorName = parameters["Interpolator"].toString();
	if (interpolatorName == InterpLinear)
	{
		typedef itk::LinearInterpolateImageFunction<InputImageType, double> InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolatorName == InterpNearestNeighbour)
	{
		typedef itk::NearestNeighborInterpolateImageFunction<InputImageType, double> InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolatorName == InterpBSpline)
	{
		typedef itk::BSplineInterpolateImageFunction<InputImageType, double> InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	else if (interpolatorName == InterpWindowedSinc)
	{
		typedef itk::Function::HammingWindowFunction<3> WindowFunctionType;
		typedef itk::ZeroFluxNeumannBoundaryCondition<InputImageType> ConditionType;
		typedef itk::WindowedSincInterpolateImageFunction<InputImageType, 3, WindowFunctionType, ConditionType, double>
			InterpolatorType;
		auto interpolator = InterpolatorType::New();
		resampler->SetInterpolator(interpolator);
	}
	resampler->SetInput(dynamic_cast<InputImageType*>(filter->imageInput(0)->itkImage()));
	resampler->SetOutputOrigin(origin);
	resampler->SetOutputSpacing(spacing);
	resampler->SetSize(size);
	resampler->SetDefaultPixelValue(0);
	filter->progress()->observe(resampler);
	resampler->Update();
	filter->addOutput(resampler->GetOutput());
}

IAFILTER_CREATE(iAResampleFilter)

void iAResampleFilter::performWork(QVariantMap const & parameters)
{
	ITK_TYPED_CALL(resampler, inputPixelType(), this, parameters);
}

iAResampleFilter::iAResampleFilter() :
	iAFilter("Resample", "Geometric Transformations",
		"Resample the image to a new size.<br/>"
		"For more information, see the "
		"<a href=\"https://itk.org/Doxygen/html/classitk_1_1ResampleImageFilter.html\">"
		"Resample Filter</a> in the ITK documentation.")
{
	addParameter("Origin", iAValueType::Vector3i, variantVector<int>({0, 0, 0}));
	addParameter("Spacing", iAValueType::Vector3, variantVector<double>({1.0, 1.0, 1.0}));
	addParameter("Size", iAValueType::Vector3i, variantVector<int>({1, 1, 1}));
	QStringList interpolators;
	interpolators
		<< InterpLinear
		<< InterpNearestNeighbour
		<< InterpBSpline
		<< InterpWindowedSinc;
	addParameter("Interpolator", iAValueType::Categorical, interpolators);
}

void iAResampleFilter::adaptParametersToInput(QVariantMap& params, std::vector<std::shared_ptr<iADataSet>> const& dataSets)
{
	assert(dataSets.size() > 0 && dynamic_cast<iAImageData*>(dataSets[0].get()));
	auto img = dynamic_cast<iAImageData*>(dataSets[0].get())->vtkImage();
	auto spc = img->GetSpacing();
	params["Spacing"] = variantVector<double>({spc[0], spc[1], spc[2]});
	auto dim     = img->GetDimensions();
	params["Size"]    = variantVector<int>({dim[0], dim[1], dim[2]});
}


template<typename T>
void extractImage(iAFilter* filter, QVariantMap const & parameters)
{
	typedef itk::Image< T, DIM > InputImageType;
	typedef itk::Image< T, DIM > OutputImageType;
	typedef itk::ExtractImageFilter< InputImageType, OutputImageType > EIFType;

	typename EIFType::InputImageRegionType::SizeType size;
	setFromVectorVariant<int>(size, parameters["Size"]);

	typename EIFType::InputImageRegionType::IndexType index;
	setFromVectorVariant<int>(index, parameters["Index"]);

	typename EIFType::InputImageRegionType region; region.SetIndex(index); region.SetSize(size);

	auto extractFilter = EIFType::New();
	extractFilter->SetInput(dynamic_cast< InputImageType * >(filter->imageInput(0)->itkImage()));
	extractFilter->SetExtractionRegion(region);
	filter->progress()->observe(extractFilter);
	extractFilter->Update();

	filter->addOutput(setIndexOffsetToZero<T>(extractFilter->GetOutput()));
}

IAFILTER_CREATE(iAExtractImageFilter)

void iAExtractImageFilter::performWork(QVariantMap const & parameters)
{
	ITK_TYPED_CALL(extractImage, inputPixelType(), this, parameters);
}

iAExtractImageFilter::iAExtractImageFilter() :
	iAFilter("Extract Image", "Geometric Transformations",
		"Extract a part of the image.<br/>"
		"Both <em>Index</em> and <em>Size</em> values are in pixel units.<br/>"
		"For more information, see the "
		"<a href=\"https://itk.org/Doxygen/html/classitk_1_1ExtractImageFilter.html\">"
		"Extract Image Filter</a> in the ITK documentation.")
{
	addParameter("Index", iAValueType::Vector3i, variantVector<int>({0, 0, 0}));
	addParameter("Size", iAValueType::Vector3i, variantVector<int>({1, 1, 1}));
}

void iAExtractImageFilter::adaptParametersToInput(QVariantMap& params, std::vector<std::shared_ptr<iADataSet>> const& dataSets)
{
	assert(dataSets.size() > 0 && dynamic_cast<iAImageData*>(dataSets[0].get()));
	adjustIndexAndSizeToImage(params, dynamic_cast<iAImageData*>(dataSets[0].get())->vtkImage());
}





template<typename T> void padImage(iAFilter* filter, QVariantMap const & parameters)
{
	typedef itk::Image< T, DIM > InputImageType;
	typedef itk::ConstantPadImageFilter<InputImageType, InputImageType> PadType;

	typename PadType::InputImageRegionType::SizeType lowerPadSize;
	lowerPadSize[0] = parameters["Lower X padding"].toUInt();
	lowerPadSize[1] = parameters["Lower Y padding"].toUInt();
	lowerPadSize[2] = parameters["Lower Z padding"].toUInt();
	typename PadType::InputImageRegionType::SizeType upperPadSize;
	upperPadSize[0] = parameters["Upper X padding"].toUInt();
	upperPadSize[1] = parameters["Upper Y padding"].toUInt();
	upperPadSize[2] = parameters["Upper Z padding"].toUInt();

	auto padFilter = PadType::New();
	padFilter->SetInput(dynamic_cast< InputImageType * >(filter->imageInput(0)->itkImage()));
	padFilter->SetPadLowerBound(lowerPadSize);
	padFilter->SetPadUpperBound(upperPadSize);
	padFilter->SetConstant(parameters["Value"].toDouble());
	filter->progress()->observe(padFilter);
	padFilter->Update();

	filter->addOutput(setIndexOffsetToZero<T>(padFilter->GetOutput()));
}

IAFILTER_CREATE(iAPadImageFilter)

void iAPadImageFilter::performWork(QVariantMap const & parameters)
{
	ITK_TYPED_CALL(padImage, inputPixelType(), this, parameters);
}

iAPadImageFilter::iAPadImageFilter() :
	iAFilter("Pad Image", "Geometric Transformations",
		"Pad image on one or more side with given number of zero pixels.<br/>"
		"<em>Lower (x, y, z) padding</em> specifies the amount of pixels to be appended before the current first x/y/z pixel. "
		"<em>Upper (x, y, z) padding</em> specifies the amount of pixels to be appended after the current last x/y/z pixel. "
		"Pixels are added with the specified <em>Value</em><br/>"
		"For more information, see the "
		"<a href=\"https://itk.org/Doxygen/html/classitk_1_1ConstantPadImageFilter.html\">"
		"Extract Image Filter</a> in the ITK documentation.")
{
	addParameter("Lower X padding", iAValueType::Discrete, 1, 0);
	addParameter("Lower Y padding", iAValueType::Discrete, 1, 0);
	addParameter("Lower Z padding", iAValueType::Discrete, 1, 0);
	addParameter("Upper X padding", iAValueType::Discrete, 1, 0);
	addParameter("Upper Y padding", iAValueType::Discrete, 1, 0);
	addParameter("Upper Z padding", iAValueType::Discrete, 1, 0);
	addParameter("Value", iAValueType::Continuous, 0.0);
}
