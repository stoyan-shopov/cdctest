#include "mainwindow.hxx"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	connect(ui->pushButtonScanPorts, SIGNAL(clicked(bool)), this, SLOT(scanPorts()));
	connect(ui->pushButtonStartTest, SIGNAL(clicked(bool)), this, SLOT(startTest()));
	connect(ui->pushButtonEndTest, SIGNAL(clicked(bool)), this, SLOT(endTest()));
	connect(ui->pushButtonPrintStatistics, SIGNAL(clicked(bool)), this, SLOT(printStatistics()));
	connect(ui->pushButtonScanPorts, SIGNAL(clicked(bool)), this, SLOT(scanPorts()));

	ui->pushButtonStartTest->setEnabled(true);
	ui->pushButtonEndTest->setEnabled(false);
	ui->pushButtonPrintStatistics->setEnabled(false);
	ui->pushButtonScanPorts->setEnabled(true);
	ui->comboBoxAvailablePorts->setEnabled(true);

	scanPorts();
	connect(& port, SIGNAL(readyRead()), this, SLOT(portReadyRead()));
	connect(& port, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this, SLOT(portError(QSerialPort::SerialPortError)));
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::startTest()
{
	if (!ui->comboBoxAvailablePorts->currentText().size())
	{
		ui->plainTextEdit->appendPlainText("No port selected");
		return;
	}
	port.setPortName(ui->comboBoxAvailablePorts->currentText());
	if (!port.open(QSerialPort::ReadWrite))
	{
		ui->plainTextEdit->appendPlainText("Failed to open serial port: " + port.errorString());
		return;
	}
	ui->plainTextEdit->appendPlainText("Test started");

	ui->pushButtonStartTest->setEnabled(false);
	ui->pushButtonEndTest->setEnabled(true);
	ui->pushButtonPrintStatistics->setEnabled(true);
	ui->pushButtonScanPorts->setEnabled(false);
	ui->comboBoxAvailablePorts->setEnabled(false);

	timer.start();
	data.clear();
	xdata.clear();
	total_count = xcount = 0;
	for (int i = 0; i < PACKET_PRELOAD_COUNT; i ++)
	{
		QByteArray d = QByteArray(USB_BLOCK_SIZE, dataChar);
		data += d;
		dataChar ++;

		if (dataChar == 'z')
			dataChar = 'a';
		else
			dataChar ++;

		if (port.write(d) == -1)
		{
			ui->plainTextEdit->appendPlainText("Error writing data");
			endTest();
			return;
		}
	}
}

void MainWindow::portReadyRead()
{
	xdata += port.readAll();
	if (!(data.startsWith(xdata)))
	{
		ui->plainTextEdit->appendPlainText(QString("Data loopback mismatch, bytes transferred: %1").arg(total_count));
		port.close();
		endTest();
		return;
	}

	while (xdata.size() >= USB_BLOCK_SIZE)
	{
		xdata = xdata.remove(0, USB_BLOCK_SIZE);
		data = data.remove(0, USB_BLOCK_SIZE);

		if (dataChar == 'z')
			dataChar = 'a';
		else
			dataChar ++;

		QByteArray d = QByteArray(USB_BLOCK_SIZE, dataChar);
		data += d;

		if (port.write(d) == -1)
		{
			ui->plainTextEdit->appendPlainText("Error writing data");
			endTest();
			return;
		}

		total_count += USB_BLOCK_SIZE;
	}
	if (total_count >= (xcount + 1) * 10 * 1024 * 1024)
	{
		xcount = total_count / (10 * 1024 * 1024);
		ui->plainTextEdit->appendPlainText(QString("%1 megabytes transferred").arg(xcount * 10));
	}
}

void MainWindow::portError(QSerialPort::SerialPortError error)
{
	if (isHandlingError || error == QSerialPort::NoError)
		return;
	isHandlingError = true;
	ui->plainTextEdit->appendPlainText(QString("Error occurred, code: %1").arg(error));
	endTest();
	isHandlingError = false;
}

void MainWindow::endTest()
{
	port.close();
	printStatistics();
	ui->plainTextEdit->appendPlainText("\nEnd of test.");

	ui->pushButtonStartTest->setEnabled(true);
	ui->pushButtonEndTest->setEnabled(false);
	ui->pushButtonPrintStatistics->setEnabled(false);
	ui->pushButtonScanPorts->setEnabled(true);
	ui->comboBoxAvailablePorts->setEnabled(true);
}

void MainWindow::printStatistics()
{
	ui->plainTextEdit->appendPlainText(QString("Checkpoint, bytes transferred: %1 in %2 seconds").arg(total_count).arg(timer.elapsed() / 1000));
	ui->plainTextEdit->appendPlainText(QString("Average speed %1 bytes/s").arg(((double) total_count * 1000) / (double) timer.elapsed()));
}

void MainWindow::scanPorts()
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	ui->comboBoxAvailablePorts->clear();
	for (const auto & p : ports)
		ui->comboBoxAvailablePorts->addItem(p.portName());
}

