#ifndef MAINWINDOW_HXX
#define MAINWINDOW_HXX

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QElapsedTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void startTest();
	void portReadyRead(void);
	void portError(QSerialPort::SerialPortError error);
	void endTest();
	void printStatistics();
	void scanPorts(void);

private:
	/* Flag to avoid recursive error handling. */
	bool isHandlingError = false;

	const unsigned USB_BLOCK_SIZE = 512;
	const int PACKET_PRELOAD_COUNT = 32;
	char dataChar = 'a';
	QByteArray data;
	QByteArray xdata;
	uint64_t total_count, xcount;
	QElapsedTimer timer;
	Ui::MainWindow *ui;
	QSerialPort port;
};
#endif // MAINWINDOW_HXX
