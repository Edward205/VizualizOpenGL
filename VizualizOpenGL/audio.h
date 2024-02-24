#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include "kiss_fft.h"
#include <vector>

class WavPlayer {
public:
    float *fftBins;

    WavPlayer(const char* filename) : filename(filename), sndfile(nullptr), stream(nullptr) {}

    bool init() {
        // Initialize PortAudio
        if (Pa_Initialize() != paNoError) {
            std::cerr << "PortAudio initialization failed" << std::endl;
            return false;
        }

        // Open the WAV file using libsndfile
        sndfile = sf_open(filename, SFM_READ, &sfinfo);
        if (!sndfile) {
            std::cerr << "Error opening WAV file" << std::endl;
            return false;
        }

        // Set up PortAudio stream parameters
        outputParameters.device = Pa_GetDefaultOutputDevice();
        outputParameters.channelCount = sfinfo.channels;
        outputParameters.sampleFormat = paFloat32;
        outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = nullptr;

        // Open PortAudio stream
        if (Pa_OpenStream(&stream, nullptr, &outputParameters, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, nullptr, nullptr) != paNoError) {
            std::cerr << "Error opening PortAudio stream" << std::endl;
            return false;
        }

        // Allocate the buffer dynamically based on sfinfo
        buffer = new float[FRAMES_PER_BUFFER * sfinfo.channels];

        // Configure KISS FFT
        fftCfg = kiss_fft_alloc(FRAMES_PER_BUFFER, 0, nullptr, nullptr);
        fftInput = new kiss_fft_cpx[FRAMES_PER_BUFFER];
        fftOutput = new kiss_fft_cpx[FRAMES_PER_BUFFER];

        fftBins = new float[FRAMES_PER_BUFFER];

        // Start PortAudio stream
        Pa_StartStream(stream);
        return true;
    }

    void play() {

        // Read and play the WAV file
        if (sf_readf_float(sndfile, buffer, FRAMES_PER_BUFFER) > 0) {
            Pa_WriteStream(stream, buffer, FRAMES_PER_BUFFER);

            // Prepare the input for FFT
            applyHanningWindow(buffer, FRAMES_PER_BUFFER);
            for (sf_count_t i = 0; i < FRAMES_PER_BUFFER; ++i) {
                fftInput[i].r = buffer[i];
                fftInput[i].i = 0.0f;
            }

            // Run FFT
            kiss_fft(fftCfg, fftInput, fftOutput);
            float maxampl = -1000.0f;
            float minampl = 1000.0f;
            for (int i = 0; i < FRAMES_PER_BUFFER / 4; ++i)
            {
                float amplitude = sqrt(fftOutput[i].r * fftOutput[i].r + fftOutput[i].i * fftOutput[i].i);
                maxampl = std::max(maxampl, amplitude);
                minampl = std::min(maxampl, amplitude);
                fftBins[i] = scaleValue(amplitude, 0.0f, 256.0f, 0.0f, 255.0f); // ???
            }
            std::cout << "max: " << maxampl << std::endl;
            std::cout << "min: " << minampl << std::endl << std::endl;

            /*for (int i = 0; i < FRAMES_PER_BUFFER; ++i) {
                float frequency = i * SAMPLE_RATE / static_cast<float>(FRAMES_PER_BUFFER);
                float amplitude = sqrt(fftOutput[i].r * fftOutput[i].r + fftOutput[i].i * fftOutput[i].i);
                std::cout << "Bin " << i << ": Frequency = " << frequency << " Hz, Amplitude = " << amplitude << std::endl;
            }*/
        }
    }

    double processAudio() {
        // Calculate the root mean square (RMS) of the audio samples
        double sumSquare = 0.0;
        for (size_t i = 0; i < FRAMES_PER_BUFFER * sfinfo.channels; ++i) {
            sumSquare += buffer[i] * buffer[i];
        }
        double rms = std::sqrt(sumSquare / (FRAMES_PER_BUFFER * sfinfo.channels));

        // Output the RMS volume level
        return rms;
    }

    void applyHanningWindow(float* data, size_t dataSize) {
        for (size_t i = 0; i < dataSize; ++i) {
            double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (dataSize - 1)));
            data[i] *= multiplier;
        }
    }

    float scaleValue(float value, float oldMin, float oldMax, float newMin, float newMax) {
        // Check for division by zero to avoid errors
        if (oldMax - oldMin == 0) {
            std::cerr << "Error: Division by zero in scaleValue function." << std::endl;
            return value;
        }

        // Scale the value to the new range
        float scaledValue = ((value - oldMin) / (oldMax - oldMin)) * (newMax - newMin) + newMin;

        return scaledValue;
    }


    ~WavPlayer() {
        // Stop and close PortAudio stream
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        
        // Release the dynamically allocated buffer
        delete[] buffer;

        // Terminate PortAudio
        Pa_Terminate();

        // Close libsndfile
        if (sndfile) {
            sf_close(sndfile);
        }

        delete fftBins;
        delete fftInput;
        delete fftOutput;
    }

private:
    const char* filename;
    SF_INFO sfinfo;
    SNDFILE* sndfile;
    PaStream* stream;
    PaStreamParameters outputParameters;
    float* buffer; // Dynamically allocated buffer
    kiss_fft_cfg fftCfg;
    kiss_fft_cpx* fftInput;
    kiss_fft_cpx* fftOutput;


    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int FRAMES_PER_BUFFER = 1024;
};

