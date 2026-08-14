class Terrain {
public:
	Terrain(float frequency, float amplitude, float gain, float lacunarity, int octaves, int max, int min, int subdivisions, int rez) {
        this->frequency = frequency;
        this->amplitude = amplitude;
        this->gain = gain;
        this->lacunarity = lacunarity;
        this->octaves = octaves;
        this->max = max;
        this->min = min;
        this->subdivisions = subdivisions;
        this->rez = rez;
	}

    // Getters
    float getFrequency() {
        return frequency;
    }

    float getAmplitude() {
        return amplitude;
    }

    float getGain() {
        return gain;
    }

    float getLacunarity() {
        return lacunarity;
    }

    int getRez() {
        return rez;
    }

    int getOctaves() {
        return octaves;
    }

    int getMax() {
        return max;
    }

    int getMin() {
        return min;
    }

    int getSubdivisions() {
        return subdivisions;
    }

    float* getFrequencyPtr() {
        return &frequency;
    }

    float* getAmplitudePtr() {
        return &amplitude;
    }

    float* getGainPtr() {
        return &gain;
    }

    float* getLacunarityPtr() {
        return &lacunarity;
    }

    int* getRezPtr() {
        return &rez;
    }

    int* getOctavesPtr() {
        return &octaves;
    }

    int* getMaxPtr() {
        return &max;
    }

    int* getMinPtr() {
        return &min;
    }

    int* getSubdivisionsPtr() {
        return &subdivisions; 
    }
    // Setters
    void setFrequency(float frequency) {
        this->frequency = frequency;
    }

    void setAmplitude(float amplitude) {
        this->amplitude = amplitude;
    }

    void setGain(float gain) {
        this->gain = gain;
    }

    void setLacunarity(float lacunarity) {
        this->lacunarity = lacunarity;
    }

    void setOctaves(int octaves) {
        this->octaves = octaves;
    }

    void setMax(int max) {
        this->max = max;
    }

    void setMin(int min) {
        this->min = min;
    }

    void setRez(int rez) {
        this->rez = rez;
    }

    void setSubdivisions(int subdivisions) {
        this->subdivisions = subdivisions;
    }

private:
    float frequency;

    float amplitude;

    float gain;

    float lacunarity;

    int octaves;

    int max;

    int min;

    int subdivisions;

    int rez;

};
