#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>

#include "dfinstance.h"

class DwarfModel;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	QSettings *get_settings() {return m_settings;}

    public slots:
        void connect_to_df();
		void read_dwarves();
		void scan_memory();
		void filter_dwarves();
		void show_toolbutton_text(bool);
		void set_group_by(int);
		void show_about();
		void add_custom_profession();

private:
    Ui::MainWindow *ui;
    DFInstance *m_df;
	QLabel *m_lbl_status;
	QSettings *m_settings;
	DwarfModel *m_model;
	bool m_reading_settings;

	void closeEvent(QCloseEvent *evt); // override;

	void read_settings();
	void write_settings();

    private slots:
        void set_interface_enabled(bool);
		
};

#endif // MAINWINDOW_H