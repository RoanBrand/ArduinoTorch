class CurrentMonitor {
public:
	CurrentMonitor();

	double GetCurrentReading(); // A

private:
	int offset = 511;
	int gain = 110; // mV/A
};