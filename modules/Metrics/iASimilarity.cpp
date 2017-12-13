/*************************************  open_iA  ************************************ *
* **********  A tool for scientific visualisation and 3D image processing  ********** *
* *********************************************************************************** *
* Copyright (C) 2016-2017  C. Heinzl, M. Reiter, A. Reh, W. Li, M. Arikan,            *
*                          J. Weissenböck, Artem & Alexander Amirkhanov, B. Fröhler   *
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
#include "pch.h"
#include "iASimilarity.h"

#include "defines.h"          // for DIM
#include "iAConnector.h"
#include "iAProgress.h"
#include "iATypedCallHelper.h"

#include <itkCastImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageToHistogramFilter.h>
#include <itkJoinImageFilter.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include <itkMeanSquaresImageToImageMetric.h>
#include <itkStatisticsImageFilter.h>
#include <itkTranslationTransform.h>

template<class T>
void similarity_metrics_template( iAProgress* p, QVector<iAConnector*> images,
	QMap<QString, QVariant> const & parameters, iAFilter* filter)
{
	typedef itk::Image< T, DIM > ImageType;
	typedef itk::TranslationTransform < double, DIM > TransformType;
	typedef itk::LinearInterpolateImageFunction<ImageType, double >	InterpolatorType;
	auto transform = TransformType::New();
	transform->SetIdentity();
	auto interpolator = InterpolatorType::New();
	interpolator->SetInputImage(dynamic_cast<ImageType *>(images[0]->GetITKImage()));
	TransformType::ParametersType params(transform->GetNumberOfParameters());

	double range = 0, imgMean = 0, imgVar = 0, refMean = 0, refVar = 0, mse = 0;
	if (parameters["Peak Signal-to-Noise Ratio"].toBool() ||
		parameters["Structural Similarity Index"].toBool() ||
		parameters["Normalized RMSE"].toBool())
	{
		typedef itk::StatisticsImageFilter<ImageType> StatisticsImageFilterType;
		auto imgStatFilter = StatisticsImageFilterType::New();
		imgStatFilter->SetInput(dynamic_cast<ImageType *>(images[0]->GetITKImage()));
		imgStatFilter->Update();
		imgMean = imgStatFilter->GetMean();
		imgVar = imgStatFilter->GetSigma();
		double imgMin = imgStatFilter->GetMinimum();
		double imgMax = imgStatFilter->GetMaximum();
		auto refStatFilter = StatisticsImageFilterType::New();
		refStatFilter->SetInput(dynamic_cast<ImageType *>(images[1]->GetITKImage()));
		refStatFilter->Update();
		refMean = refStatFilter->GetMean();
		refVar = refStatFilter->GetSigma();
		double refMin = refStatFilter->GetMaximum();
		double refMax = refStatFilter->GetMinimum();
		range = std::max(refMax, imgMax) - std::min(refMin, imgMin);
	}
	if (parameters["Mean Squared Error"].toBool() ||
		parameters["RMSE"].toBool() ||
		parameters["Normalized RMSE"].toBool() ||
		parameters["Peak Signal-to-Noise Ratio"].toBool())
	{
		typedef itk::MeanSquaresImageToImageMetric<	ImageType, ImageType > MSMetricType;
		auto msmetric = MSMetricType::New();
		msmetric->SetFixedImage(dynamic_cast<ImageType *>(images[0]->GetITKImage()));
		msmetric->SetFixedImageRegion(dynamic_cast<ImageType *>(images[0]->GetITKImage())->GetLargestPossibleRegion());
		msmetric->SetMovingImage(dynamic_cast<ImageType *>(images[1]->GetITKImage()));
		msmetric->SetTransform(transform);
		msmetric->SetInterpolator(interpolator);
		params.Fill(0.0);
		msmetric->Initialize();
		mse = msmetric->GetValue(params);
		if (parameters["Mean Squared Error"].toBool())
			filter->AddOutputValue("Mean Squared Error", mse);
		if (parameters["RMSE"].toBool())
			filter->AddOutputValue("RMSE", std::sqrt(mse));
		if (parameters["Normalized RMSE"].toBool())
			filter->AddOutputValue("Normalized RMSE", std::sqrt(mse) / range );

	}
	if (parameters["Peak Signal-to-Noise Ratio"].toBool())
	{
		double psnr = 20 * std::log10(range) - 10 * log10(mse);
		filter->AddOutputValue("Peak Signal-to-Noise Ratio", psnr);
	}
	if (parameters["Normalized Correlation"].toBool())
	{
		typedef itk::NormalizedCorrelationImageToImageMetric< ImageType, ImageType > NCMetricType;
		auto ncmetric = NCMetricType::New();
		ncmetric->SetFixedImage(dynamic_cast<ImageType *>(images[0]->GetITKImage()));
		ncmetric->SetFixedImageRegion(dynamic_cast<ImageType *>(images[0]->GetITKImage())->GetLargestPossibleRegion());
		ncmetric->SetMovingImage(dynamic_cast<ImageType *>(images[1]->GetITKImage()));
		ncmetric->SetTransform(transform);
		ncmetric->SetInterpolator(interpolator);
		params.Fill(0.0);
		ncmetric->Initialize();
		double ncVal = ncmetric->GetValue(params);
		filter->AddOutputValue("Normalized Correlation Metric", ncVal);
	}
	if (parameters["Mutual Information"].toBool())
	{
		//ITK-Example: https://itk.org/Doxygen/html/Examples_2Statistics_2ImageMutualInformation1_8cxx-example.html
		typedef itk::JoinImageFilter< ImageType, ImageType > JoinFilterType;
		auto joinFilter = JoinFilterType::New();
		joinFilter->SetInput1(dynamic_cast<ImageType *>(images[0]->GetITKImage()));
		joinFilter->SetInput2(dynamic_cast<ImageType *>(images[1]->GetITKImage()));
		joinFilter->Update();

		typedef typename JoinFilterType::OutputImageType  VectorImageType;
		typedef itk::Statistics::ImageToHistogramFilter<VectorImageType >  HistogramFilterType;
		auto histogramFilter = HistogramFilterType::New();
		histogramFilter->SetInput(joinFilter->GetOutput());
		histogramFilter->SetMarginalScale(10.0);
		typedef typename HistogramFilterType::HistogramSizeType   HistogramSizeType;
		HistogramSizeType size(2);
		size[0] = parameters["Histogram Bins"].toDouble();  // number of bins for the first  channel
		size[1] = parameters["Histogram Bins"].toDouble();  // number of bins for the second channel
		histogramFilter->SetHistogramSize(size);
		/*
		typedef typename HistogramFilterType::HistogramMeasurementVectorType HistogramMeasurementVectorType;
		HistogramMeasurementVectorType binMinimum(3);
		HistogramMeasurementVectorType binMaximum(3);
		// shouldn't this be the min and max DATA values?
		binMinimum[0] = -0.5;
		binMinimum[1] = -0.5;
		binMinimum[2] = -0.5;
		binMaximum[0] = miHistoBins + 0.5;
		binMaximum[1] = miHistoBins + 0.5;
		binMaximum[2] = miHistoBins + 0.5;
		histogramFilter->SetHistogramBinMinimum(binMinimum);
		histogramFilter->SetHistogramBinMaximum(binMaximum);
		*/
		histogramFilter->SetAutoMinimumMaximum(true);
		histogramFilter->Update();
		typedef typename HistogramFilterType::HistogramType  HistogramType;
		const HistogramType * histogram = histogramFilter->GetOutput();
		auto itr = histogram->Begin();
		auto end = histogram->End();
		const double Sum = histogram->GetTotalFrequency();
		double jointEntr = 0;
		while (itr != end)
		{
			const double count = itr.GetFrequency();
			if (count > 0.0)
			{
				const double probability = count / Sum;
				jointEntr += -probability * std::log(probability) / std::log(2.0);
			}
			++itr;
		}

		size[0] = parameters["Histogram Bins"].toDouble();  // number of bins for the first  channel
		size[1] = 1;  // number of bins for the second channel
		histogramFilter->SetHistogramSize(size);
		histogramFilter->Update();
		itr = histogram->Begin();
		end = histogram->End();
		double entr1 = 0;
		while (itr != end)
		{
			const double count = itr.GetFrequency();
			if (count > 0.0)
			{
				const double probability = count / Sum;
				entr1 += -probability * std::log(probability) / std::log(2.0);
			}
			++itr;
		}

		size[0] = 1;  // number of bins for the first channel
		size[1] = parameters["Histogram Bins"].toDouble();  // number of bins for the second channel
		histogramFilter->SetHistogramSize(size);
		histogramFilter->Update();
		itr = histogram->Begin();
		end = histogram->End();
		double entr2 = 0;
		while (itr != end)
		{
			const double count = itr.GetFrequency();
			if (count > 0.0)
			{
				const double probability = count / Sum;
				entr2 += -probability * std::log(probability) / std::log(2.0);
			}
			++itr;
		}
		double mutInf = entr1 + entr2 - jointEntr;
		double norMutInf1 = 2.0 * mutInf / (entr1 + entr2);
		double norMutInf2 = (entr1 + entr2) / jointEntr;
		filter->AddOutputValue("Image 1 Entropy", entr1);
		filter->AddOutputValue("Image 2 Entropy", entr2);
		filter->AddOutputValue("Joint Entropy", jointEntr);
		filter->AddOutputValue("Mutual Information", mutInf);
		filter->AddOutputValue("Normalized Mutual Information 1", norMutInf1);
		filter->AddOutputValue("Normalized Mutual Information 2", norMutInf2);
	}
	if (parameters["Structural Similarity Index"].toBool())
	{
		ImageType* img = dynamic_cast<ImageType *>(images[0]->GetITKImage());
		ImageType* ref = dynamic_cast<ImageType *>(images[1]->GetITKImage());
		itk::ImageRegionConstIterator<ImageType> imgIt(img, img->GetLargestPossibleRegion());
		itk::ImageRegionConstIterator<ImageType> refIt(ref, ref->GetLargestPossibleRegion());
		imgIt.GoToBegin(); refIt.GoToBegin();
		double covSum = 0;	size_t count = 0;
		while (!imgIt.IsAtEnd() && !refIt.IsAtEnd())
		{
			covSum += (imgIt.Get() - imgMean) * (refIt.Get() - refMean);
			++imgIt; ++refIt; ++count;
		}
		double covariance = covSum / count;
		double c1 = std::pow(parameters["Structural Similarity k1"].toDouble() * range, 2);
		double c2 = std::pow(parameters["Structural Similarity k2"].toDouble() * range, 2);
		double ssim = ((2 * imgMean * refMean + c1) * (2 * covariance + c2)) /
			((imgMean * imgMean + refMean * refMean + c1) * (imgVar + refVar + c2));
		filter->AddOutputValue("Structural Similarity Index", ssim);
	}
}

iASimilarity::iASimilarity() : iAFilter("Similarity", "Metrics",
	"Calculates the similarity between two images according to different metrics.<br/>"
	"<strong>NOTE</strong>: Normalize the images before calculating the similarity metrics!<br/>"
	"<a href=\"https://itk.org/Doxygen/html/ImageSimilarityMetricsPage.html\">General information on ITK similarity metrics</a>.<br/>"
	"<em><a href=\"https://itk.org/Doxygen/html/classitk_1_1MeanSquaresImageToImageMetric.html\">"
	"Mean Squared Error (MSE) Metric</a></em>: The optimal value of the metric is zero, which means that the two input images are equal. "
	"Poor matches between images A and B result in large values of the metric. This metric relies on the assumption that intensity "
	"representing the same homologous point must be the same in both images.<br/>"
	"<em>RMSE</em> (Root Mean Square Error) yields the square root of the MSE, which is the mean absolute difference in intensity, "
	"which is a more intuitive measure for difference as it is in the same unit as the intensity values of the image. "
	"The <em>Normalized RMSE</em> yields a value between 0 and 1, where 0 signifies that the images are equal, "
	"and 1 that the images are as different as possible (that is, that they have the maximum possible difference at each point). "
	"It is calculated by dividing the RMSE by the maximum possible difference.<br/>"
	"The <em>Peak Signal-to-Noise Ratio</em> is computed as 10 * log10(max_intensity² / MSE), where MSE is the Mean Squared Error, "
	"and max_intensity is the maximum possible intensity difference between the two specified images.<br/>"
	"<a href=\"https://itk.org/Doxygen/html/classitk_1_1NormalizedCorrelationImageToImageMetric.html\">"
	"Normalized Correlation Metric</a>: Note the −1 factor in the metric computation. This factor is used to make the "
	"metric be optimal when its minimum is reached.The optimal value of the metric is then minus one. Misalignment "
	"between the images results in small measure values.<br/>"
	"More Information on Mutual Information is given in the "
	"<a href=\"https://itk.org/ItkSoftwareGuide.pdf\">ITK Software Guide</a> in the sections '3.10.4 Mutual "
	"Information Metric' (pp. 262-264) and '5.3.2 Information Theory' (pp. 462-471)."
	"The <em>Structural Similarity Index</em> Metric (SSIM) is a metric calculated from mean, variance and covariance "
	"of the two compared images. For more details see e.g. the "
	"<a href=\"https://en.wikipedia.org/wiki/Structural_similarity\">Structural Similarity index article in wikipedia</a>, "
	"the two parameters k1 and k2 are used exactly as defined there.",
	2, 0)
{
	AddParameter("Mean Squared Error", Boolean, false);
	AddParameter("RMSE", Boolean, true);
	AddParameter("Normalized RMSE", Boolean, false);
	AddParameter("Peak Signal-to-Noise Ratio", Boolean, true);
	AddParameter("Normalized Correlation", Boolean, false);
	AddParameter("Mutual Information", Boolean, false);
	AddParameter("Histogram Bins", Discrete, 256, 2);
	AddParameter("Structural Similarity Index", Boolean, true);
	AddParameter("Structural Similarity k1", Continuous, 0.01);
	AddParameter("Structural Similarity k2", Continuous, 0.03);
}

IAFILTER_CREATE(iASimilarity)

void iASimilarity::PerformWork(QMap<QString, QVariant> const & parameters)
{
	ITK_TYPED_CALL(similarity_metrics_template, m_con->GetITKScalarPixelType(), m_progress, m_cons, parameters, this);
}
