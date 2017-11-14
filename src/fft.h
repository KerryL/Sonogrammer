/*=============================================================================
                                   LibPlot2D
                       Copyright Kerry R. Loux 2011-2016

                  This code is licensed under the GPLv2 License
                    (http://opensource.org/licenses/GPL-2.0).
=============================================================================*/

// File:  fft.h
// Date:  5/3/2011
// Auth:  K. Loux
// Desc:  Performs Fast Fourier Transform on data.

#ifndef FFT_H_
#define FFT_H_

// Standard C++ headers
#include <string>
#include <set>
#include <vector>
#include <memory>

// Loal forward declarations
class Dataset2D;

/// Class for performing FFTs and related operations.
class FastFourierTransform
{
public:
	/// Enumeration of available FFT window functions
	enum class WindowType
	{
		Uniform,///< For use with transients whose duration is less than the length of the window
		Hann,///< For use in general-purpose applications
		Hamming,///< For use with closely-spaced sine waves
		FlatTop,///< For use in accurate single-tone amplitude measurements
		//Force,
		Exponential,///< For use with response measurements (i.e. response to an impact test)
		Count///< Number of windows available
	};

	/// Computes FFT of the specified dataset with default options.  By
	/// default, the sample is chopped so the size is equal to the next-lowest
	/// power of two.  No averaging is used.  A Hann window is applied and the
	/// data is mean-subtracted.
	///
	/// \param data Data set for which FFT should be computed.
	///
	/// \return The processed amplitude vs. frequency FFT information.
	static std::unique_ptr<Dataset2D> ComputeFFT(const Dataset2D& data);

	/// Computes FFT of the specified dataset with specified options.
	/// Averaging is determined based on the length of the signal, the
	/// specified window size and overlap.
	///
	/// \param data         Data for which FFT should be computed.
	/// \param window       Window function to be applied.
	/// \param windowSize   Length of the window.
	/// \param overlap      Overlap between adjacent windows in percent (0.0 to
	///                     1.0).
	/// \param subtractMean Indicates whether or not the data should be
	///                     mean-subtracted to remove DC content.
	///
	/// \return The processed amplitude vs. frequency FFT information.
	///
	/// \see GetNumberOfAverages
	static std::unique_ptr<Dataset2D> ComputeFFT(Dataset2D data,
		const WindowType &window, unsigned int windowSize,
		const double &overlap, const bool &subtractMean);

	/// Returns the number of averages to be used given the specified window
	/// parameters.
	///
	/// \param windowSize Number of points in each window.
	/// \param overlap    Overlap in percent (0.0 to 1.0) between adjacent
	///                   windows.
	/// \param dataSize   Number of points in signal time history.
	///
	/// \return The number of averages.
	static unsigned int GetNumberOfAverages(const unsigned int windowSize,
		const double &overlap, const unsigned int &dataSize);

	/// Returns the percent overlap between adjacent windows, given the
	/// specified window parameters.
	///
	/// \param windowSize       Number of points in each window.
	/// \param numberOfAverages Number of averages to use.
	/// \param dataSize         Number of points in signal time history.
	///
	/// \return The percent overlap between adjacent windows.
	static double ComputeOverlap(unsigned int &windowSize,
		unsigned int &numberOfAverages, const unsigned int &dataSize);

	/// Returns a string containing the name of the specified window.
	///
	/// \param window Type of window.
	///
	/// \return The name of the window.
	static std::string GetWindowName(const WindowType &window);

	/// Returns the largest power of two that is smaller than the specified
	/// sample size.
	///
	/// \param sampleSize The number of points in the sample.
	///
	/// \return The largest allowable power of two for the specified sample
	///         size.
	static unsigned int GetMaxPowerOfTwo(const unsigned int &sampleSize);

private:
	static void ApplyWindow(Dataset2D &data, const WindowType &window);
	static void ApplyHannWindow(Dataset2D &data);
	static void ApplyHammingWindow(Dataset2D &data);
	static void ApplyFlatTopWindow(Dataset2D &data);
	//static void ApplyForceWindow(Dataset2D &data);
	static void ApplyExponentialWindow(Dataset2D &data);

	static void DoBitReversal(Dataset2D &set);
	static void DoFFT(Dataset2D &temp);

	static void ZeroDataset(Dataset2D &data);
	static Dataset2D GenerateConstantDataset(const double &xValue, const double &yValue, const unsigned int &size);

	static Dataset2D ConvertDoubleSidedToSingleSided(const Dataset2D &fullSpectrum, const bool &preserveDCValue = true);

	static Dataset2D ChopSample(const Dataset2D &data, const unsigned int &sample,
		const unsigned int &windowSize, const double &overlap);

	static void AddToAverage(Dataset2D &average, const Dataset2D &data, const unsigned int &count);

	static void ConvertAmplitudeToDecibels(Dataset2D &fft);

	static void PopulateFrequencyData(Dataset2D &data, const double &sampleRate);

	static Dataset2D GetAmplitudeData(const Dataset2D &rawFFT, const double &sampleRate);

	static Dataset2D ComputeRawFFT(const Dataset2D &data, const WindowType &window);
	static void InitializeRawFFTDataset(Dataset2D &rawFFT, const Dataset2D &data, const WindowType &window);

	static Dataset2D ComplexAdd(const Dataset2D &a, const Dataset2D &b);
	static Dataset2D ComplexMultiply(const Dataset2D &a, const Dataset2D &b);
	static Dataset2D ComplexDivide(const Dataset2D &a, const Dataset2D &b);
	static Dataset2D ComplexMagnitude(const Dataset2D &a);
	static Dataset2D ComplexPower(const Dataset2D &a, const double &power);

	static unsigned int ComputeRequiredOverlapPoints(const unsigned int &dataSize,
		const unsigned int &windowSize, const unsigned int &averages);
};

#endif// FFT_H_
