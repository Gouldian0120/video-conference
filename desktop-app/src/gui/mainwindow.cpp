#include "new_conference_wizard.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    

    ui->myCameraView->setScene(new QGraphicsScene(this));
    ui->myCameraView->scene()->addItem(&pixmap);

    new_conf_wizard = std::make_shared<NewConferenceWizard>();
    // Connect finish signal from Conference Wizard
    connect(new_conf_wizard.get(), SIGNAL(newConferenceJoined()), this, SLOT(startConference()));

    // Connect buttons
    connect(ui->infoBtn, SIGNAL(released()), this, SLOT(showAboutBox()));
    connect(ui->startStopBtn, SIGNAL(released()), this, SLOT(newConference()));

    // Option selector events
    connect(ui->cameraSelector, SIGNAL(activated(int)), this,
            SLOT(cameraSelector_activated()));


    refreshCams();

    // Init Audio
    SDL_Init(SDL_INIT_AUDIO);

}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::playShutter() {
    SDL_AudioSpec wavSpec;
	Uint32 wavLength;
	Uint8 *wavBuffer;

	SDL_LoadWAV("sounds/shutter-fast.wav", &wavSpec, &wavBuffer, &wavLength);
	
	// open audio device
	SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);

	// play audio
	int success = SDL_QueueAudio(deviceId, wavBuffer, wavLength);
	SDL_PauseAudioDevice(deviceId, 0);

	// keep window open enough to hear the sound
	SDL_Delay(200);

	// clean up
	SDL_CloseAudioDevice(deviceId);
	SDL_FreeWAV(wavBuffer);
}



void MainWindow::newConference() {

    VideoStreamService &ss = VideoStreamService::instance();

    if (ss.isStreaming()) {
        ss.stopStreaming();
        ui->startStopBtn->setText("Join a Conference");
    } else {
        new_conf_wizard->restart();
        new_conf_wizard->show();
    }

}



void MainWindow::startConference() {
    ui->startStopBtn->setText("Leave Conference");
}


void MainWindow::cameraSelector_activated() {
    selected_camera_index =
        ui->cameraSelector
            ->itemData(ui->cameraSelector->currentIndex())
            .toInt();
}


void MainWindow::showAboutBox() {
    QMessageBox::about(this, "About Us",
                       "iCT Video Conference - Desktop App\n"
                       "Authors:\n"
                       "\t- Nguyen Viet Anh \n"
                       "\t- Nguyen Dinh Tuan\n"
                       "\t- Nguyen Sy An\n"
                       "Icons made by:\n"
                       "\t- https://www.flaticon.com/authors/smashicons\n"
                       "\t- https://www.flaticon.com/authors/roundicons\n"
                       "\t- https://www.freepik.com/free-vector/bunny-ears-nose-carnival-mask-photo_4015599.htm\n"
                       "\t- https://www.freepik.com/\n"
                       "\t- https://www.flaticon.com/authors/vectors-market/\n"
                       );
}

void MainWindow::showCam() {
    using namespace cv;

    // refreshCams();
    // if (!video.open(selected_camera_index)) {
    //     selected_camera_index = -1;
    //     if (!video.open(selected_camera_index)) {
    //         QMessageBox::critical(
    //             this, "Camera Error",
    //             "Make sure you entered a correct camera index,"
    //             "<br>or that the camera is not being accessed by another program!");
    //         return;
    //     }
    // }

    // current_camera_index = selected_camera_index;

    // Mat frame;
    // while (true) {

    //     // User changed camera
    //     if (selected_camera_index != current_camera_index) {
            
    //         video.release();
    //         refreshCams();
    //         current_camera_index = selected_camera_index;
    //         video.open(current_camera_index);

    //     } else if (!video.isOpened()) {

    //         // Reset to default camera (0)
    //         refreshCams();
    //         current_camera_index = selected_camera_index =
    //         ui->cameraSelector
    //             ->itemData(ui->cameraSelector->currentIndex())
    //             .toInt();
    //         ui->cameraSelector->setCurrentIndex(0);
    //         video.open(current_camera_index);

    //     }

    //     // If we still cannot open camera, exit the program
    //     if (!video.isOpened()) {
    //         QMessageBox::critical(
    //         this, "Camera Error",
    //         "Make sure you entered a correct camera index,"
    //         "<br>or that the camera is not being accessed by another program!");
    //         exit(1);
    //     }

    //     video >> frame;
    //     if (!frame.empty()) {

    //         // Flip frame
    //         if (ui->flipCameraCheckBox->isChecked()) {
    //             flip(frame, frame, 1);
    //         }

    //         setCurrentImage(frame);

    //         // ### Show current image
    //         QImage qimg(frame.data, static_cast<int>(frame.cols),
    //                     static_cast<int>(frame.rows),
    //                     static_cast<int>(frame.step), QImage::Format_RGB888);
    //         pixmap.setPixmap(QPixmap::fromImage(qimg.rgbSwapped()));
    //         ui->myCameraView->fitInView(&pixmap, Qt::KeepAspectRatio);
    //     }
    //     qApp->processEvents();
    // }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (video.isOpened()) {
        video.release();
    }
    QApplication::quit();
    exit(0);
}

void MainWindow::refreshCams() {

    // Get the number of camera available
    cv::VideoCapture temp_camera;
    ui->cameraSelector->clear();
    for (int i = 0; i < MAX_CAMS; ++i) {
        cv::VideoCapture temp_camera(i);
        bool fail = (!temp_camera.isOpened());
        temp_camera.release();

        // If we can open camera, add new camera to list
        if (!fail) {
            ui->cameraSelector->addItem(
            QString::fromUtf8((std::string("CAM") + std::to_string(i)).c_str()),
            QVariant(static_cast<int>(i)));
        }
    }

    // Select current camera
    int index = ui->cameraSelector->findData(selected_camera_index);
    if ( index != -1 ) { // -1 for not found
        ui->cameraSelector->setCurrentIndex(index);
    }
}

void MainWindow::setCurrentImage(const cv::Mat &img) {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    current_img = img.clone();
}

cv::Mat MainWindow::getCurrentImage() {
    std::lock_guard<std::mutex> guard(current_img_mutex);
    return current_img.clone();
}