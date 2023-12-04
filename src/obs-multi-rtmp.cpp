#include "pch.h"

#include <list>
#include <regex>
#include <filesystem>
#include <vector>
#include "push-widget.h"

#include "output-config.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#define ConfigSection "obs-multi-rtmp"

static class GlobalServiceImpl : public GlobalService {
public:
	bool RunInUIThread(std::function<void()> task) override
	{
		if (uiThread_ == nullptr)
			return false;
		QMetaObject::invokeMethod(
			uiThread_, [func = std::move(task)]() { func(); });
		return true;
	}

	QThread *uiThread_ = nullptr;
} s_service;

GlobalService &GetGlobalService()
{
	return s_service;
}

class MultiOutputWidget : public QDockWidget {
	int dockLocation_;
	bool dockVisible_;
	bool reopenShown_;

	// QLabel *StLabel_;
	// QLabel *sloganLabel_;
	// QLabel *codeLabel_;
	// QLineEdit *uidLineEdit_;
    // QLineEdit *keyLineEdit_;
	// QPushButton *verifyButton_;
	QNetworkAccessManager *networkManager_;
	QNetworkReply *networkReply;
	QJsonDocument jsonDoc;
	QJsonObject jsonObj;

public:
	MultiOutputWidget(QWidget *parent = 0)
		: QDockWidget(parent), reopenShown_(false)
	{
		setWindowTitle(obs_module_text("Streamway"));
		setFeatures(QDockWidget::DockWidgetFloatable |
			    QDockWidget::DockWidgetMovable);

		// save dock location
		QObject::connect(this, &QDockWidget::dockLocationChanged,
				 [this](Qt::DockWidgetArea area) {
					 dockLocation_ = area;
				 });

		scroll_ = new QScrollArea(this);
		scroll_->move(0, 22);

		container_ = new QWidget(this);
		layout_ = new QVBoxLayout(container_);
		layout_->setAlignment(Qt::AlignmentFlag::AlignTop);

		layout_->addWidget(LoginWidget());
		// StLabel_ = new QLabel("Streamway", container_);
		// layout_->addWidget(StLabel_);

		// sloganLabel_ = new QLabel("Get More views By multistreaming Directly from OBS", container_);
		// layout_->addWidget(sloganLabel_);

		// This part is a after login
				// Create a scroll area and set up a widget to contain the items
		// QScrollArea* scrollArea = new QScrollArea;
		// QWidget* scrollWidget = new QWidget;
		// // scrollArea->setWidgetResizable(true);
		// scrollWidget->setFixedSize(330, 500);
		// // Create a layout for the widget inside the scroll area
		// QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
		// layout_->addWidget(scrollWidget);
		// scrollWidget->setVisible(false);

		// // Add a label and text box for entering the uid
		// codeLabel_ = new QLabel("Uid", container_);
		// layout_->addWidget(codeLabel_);

		// uidLineEdit_ = new QLineEdit(container_);
		// layout_->addWidget(uidLineEdit_);

        // // Add a label and text box for entering the code
		// codeLabel_ = new QLabel("API Key", container_);
		// layout_->addWidget(codeLabel_);

		// keyLineEdit_ = new QLineEdit(container_);
		// layout_->addWidget(keyLineEdit_);

		// // Add a button for verification
		// verifyButton_ = new QPushButton("Verify", container_);
		// layout_->addWidget(verifyButton_);

		// QObject::connect(verifyButton_, &QPushButton::clicked, [this, scrollLayout , scrollWidget]() {
		// 	// Get the code entered by the user
		// 	QString uid = uidLineEdit_->text();
        //     QString key = keyLineEdit_->text();
		// 	QString combined = uid + ":" + key;
		// 	// Construct the request
		// 	QNetworkRequest request(QUrl("http://localhost:8000/v1/destinations"));
		// 	request.setHeader(QNetworkRequest::ContentTypeHeader,
		// 			  "application/json");

		// 	// Create a QNetworkAccessManager for making the HTTP request
		// 	networkManager_ = new QNetworkAccessManager(this);

		// 	QByteArray combinedData = combined.toUtf8().toBase64();
		// 	QString base64AuthHeader = "Basic " + QString(combinedData);
		// 	request.setRawHeader("Authorization", base64AuthHeader.toUtf8());
		// 	// Send the POST request
		// 	networkReply = networkManager_->get(request);

		// 	// Connect to the finished signal to handle the response
		// 	QObject::connect(networkReply, &QNetworkReply::finished, [this, scrollLayout ,scrollWidget]() {
		// 		if (networkReply->error() ==
		// 		    QNetworkReply::NoError) {
		// 			// Successful response received

		// 			// Read the response data
		// 			QByteArray responseData =
		// 				networkReply->readAll();

		// 			// TODO: Parse the response data to extract the token
		// 			// Assuming the response contains a JSON object with a "token" field
		// 			jsonDoc = QJsonDocument::fromJson(
		// 				responseData);
		// 			jsonObj = jsonDoc.object();

		// 			if (jsonObj.contains("token")) {
		// 				QString token =
		// 					jsonObj["token"]
		// 						.toString();

		// 				// You have a valid token
		// 				// You can use 'token' as needed
		// 				qDebug() << "Token: " << token;
		// 				handleTab(token , layout_ , scrollLayout);
		// 				// handleSuccessfulLogin(token ,  scrollLayout);
		// 				// Hide the verification UI
        //     			 uidLineEdit_->setVisible(false);
    	// 				 keyLineEdit_->setVisible(false);
    	// 				 verifyButton_->setVisible(false);
		// 				 codeLabel_->setVisible(false);
		// 				 scrollWidget->setVisible(true);
		// 			} else {
		// 				// No token found in the response, indicating an invalid code
		// 				qDebug() << "Invalid token";
		// 				QLabel *codeLabela_ = new QLabel(
		// 					"Invalid token:",
		// 					container_);
		// 				layout_->addWidget(codeLabela_);
		// 			}
		// 		} else {
		// 			// Handle the network error here
		// 			qDebug() << "Network error: "
		// 				 << networkReply->errorString();
		// 			QLabel *codeLabelc_ = new QLabel(
		// 				networkReply->errorString(),
		// 				container_);
		// 			layout_->addWidget(codeLabelc_);
		// 		}

		// 		// Clean up the network resources
		// 		networkReply->deleteLater();
			// });
		// });



		// donate
		if (std::string(
			    "\xe5\xa4\x9a\xe8\xb7\xaf\xe6\x8e\xa8\xe6\xb5\x81") ==
		    obs_module_text("Title")) {
			auto cr = new QWidget(container_);
			auto innerLayout = new QGridLayout(cr);
			innerLayout->setAlignment(Qt::AlignmentFlag::AlignLeft);

			auto label = new QLabel(
				u8"免费领红包或投喂支持插件作者。", cr);
			innerLayout->addWidget(label, 0, 0, 1, 2);
			innerLayout->setColumnStretch(0, 4);
			auto label2 = new QLabel(u8"作者：雷鸣", cr);
			innerLayout->addWidget(label2, 1, 0, 1, 1);
			auto btnFeed = new QPushButton(u8"支持", cr);
			innerLayout->addWidget(btnFeed, 1, 1, 1, 1);

			QObject::connect(btnFeed, &QPushButton::clicked, [this]() {
				const char redbagpng[] =
					"iVBORw0KGgoAAAANSUhEUgAAAJgAAACXAQMAAADTWgC3AAAABlBMVEUAAAD///+l2Z/dAAAAAWJLR0Q"
					"AiAUdSAAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAWtJREFUSMe1lk2OgzAMhY1YZJkj5CbkYkggcTG4SY"
					"6QZRaonmcHqs7PYtTaVVWSLxJu7JfnEP/+0H9ZIaKRA0aZz4QJJXuGQFsJO9HU104H1ihuTENl4IS12"
					"YmVcFSa4unJuE2xZV69mOav5XrX6nRgMi6Ii+3Nr9p4k8m7w5OtmOVbzw8ZrOmbxs0Y/ktENMlfnQnx"
					"NweG2vB1ZrZCPoyPfmbwXWSPiz1DvIrHwOHgFQsQtTkrmG6McRvqUu4aGbM28Cm1wRM62HtOP2DwkFF"
					"ypKVoU/dEa8Y9rtaFJLg5EzscoSfBKMWgZ8aY9bj4EQ1jo9GDIR68kKukMCF/6pPWTPW0R9XulVNzpj"
					"6Z5ZzMpOZrzvRElPC49Awx2LOi3k7aP+akhnL1AEMmPYphvtqeGD032TPt5zB2kQBq5Mgo9hrl7lceT"
					"MQsEkD80YH1O9xRw9Vzn/cSQ6Y6EK1JH3nVxvss/GCf3L3/YF97Nxv6vuoIAwAAAABJRU5ErkJggg==";
				const char alipaypng[] =
					"iVBORw0KGgoAAAANSUhEUgAAALsAAAC4AQMAAACByg+HAAAABlBMVEUAAAD///+l2Z/dAAAAAWJLR0Q"
					"AiAUdSAAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAVtJREFUWMPNmEGugzAMRM0qx+CmIbkpx8gK1zM2/U"
					"L66w4RogqvC5dhxqbm/6/TfgEuwzr8PNwnjhP7pgYNxR1r97XPhUvxjUsPztj0hkJ7O7c4m/V3gCg36"
					"r5Q68uA7SHtaC8BlBbP2T4awFNzEaANOtSt4+EPEciEiKJn2eCZJSIQ5XbU5yFt+HNUfIgBNqFo6Grh"
					"BDxttIQcoL5ICrsTbdAYWgBPIltxK7uVRaeLQToTFRMbb+JoYoAHjsG63UWXtFIQm2w/mV9x9vodUnC"
					"vCWfuiA+oqwaLH7Al8qL/aS4GA9I6zkz/bbFBSsHVKi++Dftg89YC53DDq8iLTJCSVgcq6DlMZGRkzm"
					"pBtW2jRRH3flU3UIIacWBLduv0gxzwltGTaUtOFS4HVSJHnBp35jvAsbJnV/RbewWgtLVyAlODkjZSd"
					"eMrkNfsIwX3awYfuLLEaGKg/OvlAz+wXVruSNSgAAAAAElFTkSuQmCC";
				const char wechatpng[] =
					"iVBORw0KGgoAAAANSUhEUgAAAK8AAACtAQMAAAD8lL09AAAABlBMVEUAAAD///+l2Z/dAAAAAWJLR0Q"
					"AiAUdSAAAAAlwSFlzAAAuIwAALiMBeKU/dgAAAbpJREFUSMe9l0GugzAMRI26yDJHyE3Si1WiEhcrN8"
					"kRWLJA8Z9xaNUv/eUfUIXgqQtn4pkY87+ubv+Cd8O1T3g2y4unzveqxXf3BniUeLKa3XcxrnZr+32zg"
					"iXPDvy0S/C04WYo4jpsKCK97FH2K3DovfqzoJzAX9sgwtFVNS/tvH03mwYPs6zb7PjDrf22lAZT6ugq"
					"wwtaCzLYWLwO13yUUQl3N70yDSTFWOqCJbORsdloLYguxrSNgxyWhl3Z0l2LjWaZUE541qaRFUqM342"
					"5sDTYdY5EZE1KjHU/z25+0UV3yi/GE4LeubHe0V9QYFZjEOhN70QOmp0JocSdGT82Nh+D7nrcQoHkDO"
					"GOcmLnhXil3vAsy5nRWhS9SjGkhmMiddHIMO6580oMu5a0xpAC0aOSOHd0mC+jodjNnhyVqDH1Tj3Ts"
					"0hEm3CGv7dYhDkdQGr0cOJxxquMM02GI44g+tKiqwwZVd4HugjHfOKxeEYvQ9jVOKawzrRnQkQSf4Yz"
					"EeasiWHhwfYNF8WkIscc985pKCaGSzA9S6eGAmpMvRlFzonBLZ6qFp8nyhxSM/IP+3x2arDwc/kHxnM"
					"tm62qBBUAAAAASUVORK5CYII=";
				auto donateWnd = new QDialog();
				donateWnd->setWindowTitle(u8"赞助");
				auto redbagBtn = new QPushButton(
					u8"支付宝领红包", donateWnd);
				auto redbagQr = new QLabel(donateWnd);
				auto redbagQrBmp =
					QPixmap::fromImage(QImage::fromData(
						QByteArray::fromBase64(
							QByteArray::fromRawData(
								redbagpng,
								sizeof(redbagpng) -
									1)),
						"png"));
				redbagQr->setPixmap(redbagQrBmp);
				auto aliBtn =
					new QPushButton(u8"支付宝", donateWnd);
				auto aliQr = new QLabel(donateWnd);
				auto aliQrBmp =
					QPixmap::fromImage(QImage::fromData(
						QByteArray::fromBase64(
							QByteArray::fromRawData(
								alipaypng,
								sizeof(alipaypng) -
									1)),
						"png"));
				aliQr->setPixmap(aliQrBmp);
				auto weBtn =
					new QPushButton(u8"微信", donateWnd);
				auto weQr = new QLabel(donateWnd);
				auto weQrBmp = QPixmap::fromImage(QImage::fromData(
					QByteArray::fromBase64(
						QByteArray::fromRawData(
							wechatpng,
							sizeof(wechatpng) - 1)),
					"png"));
				weQr->setPixmap(weQrBmp);
				auto layout = new QGridLayout(donateWnd);
				layout->addWidget(redbagBtn, 0, 0);
				layout->addWidget(aliBtn, 0, 1);
				layout->addWidget(weBtn, 0, 2);
				layout->addWidget(redbagQr, 1, 0, 1, 3);
				layout->addWidget(aliQr, 1, 0, 1, 3);
				layout->addWidget(weQr, 1, 0, 1, 3);
				aliQr->setVisible(false);
				weQr->setVisible(false);
				QObject::connect(
					redbagBtn, &QPushButton::clicked,
					[redbagQr, aliQr, weQr]() {
						redbagQr->setVisible(true);
						aliQr->setVisible(false);
						weQr->setVisible(false);
					});
				QObject::connect(
					aliBtn, &QPushButton::clicked,
					[redbagQr, aliQr, weQr]() {
						redbagQr->setVisible(false);
						aliQr->setVisible(true);
						weQr->setVisible(false);
					});
				QObject::connect(
					weBtn, &QPushButton::clicked,
					[redbagQr, aliQr, weQr]() {
						redbagQr->setVisible(false);
						aliQr->setVisible(false);
						weQr->setVisible(true);
					});
				donateWnd->setLayout(layout);
				donateWnd->exec();
			});

			layout_->addWidget(cr);
			// auto stackedLayout = new QStackedLayout;
		} else {

		auto imageLabel = new QLabel(container_);
		QPixmap pixmap("https://localhost:3000/src/Images/icon.png");
		imageLabel->setPixmap(pixmap);



			auto horizontalLayout = new QHBoxLayout;
			auto label = new QLabel(
				u8"<p>Get Your <a href=\"https://app.streamway.in/setting\">Uid and Api Key</a></p>",
				container_);
			label->setTextFormat(Qt::RichText);
			label->setTextInteractionFlags(
				Qt::TextBrowserInteraction);
			label->setOpenExternalLinks(true);
			horizontalLayout->addWidget(label);

			auto watchLable = new QLabel(
				u8"<p>How to Use <a href=\"https://app.streamway.in/setting\">Watch Our Video</a> </p>",
				container_);
			watchLable->setTextFormat(Qt::RichText);
			watchLable->setTextInteractionFlags(
				Qt::TextBrowserInteraction);
			watchLable->setOpenExternalLinks(true);
			horizontalLayout->addWidget(watchLable);

			auto supportLable = new QLabel(
				u8"<p>Any Help <a href=\"https://streamway.in/contact\">Contact Us</a></p>",
				container_);
			supportLable->setTextFormat(Qt::RichText);
			supportLable->setTextInteractionFlags(
				Qt::TextBrowserInteraction);
			supportLable->setOpenExternalLinks(true);
			horizontalLayout->addWidget(supportLable);
			layout_->addLayout(horizontalLayout);
			label->setWordWrap(true);
			watchLable->setWordWrap(true);
			supportLable->setWordWrap(true);
		}

		scroll_->setWidgetResizable(true);
		scroll_->setWidget(container_);

		setLayout(layout_);

		// resize(200, 400);
	}

// Function to create Tab 1 and its content
QWidget* LoginWidget() {
    QWidget* loginWidget = new QWidget;
    QVBoxLayout* LoginLayout = new QVBoxLayout(loginWidget);

	// QLabel* StLabel_ = new QLabel("Streamway", loginWidget);
	// LoginLayout->addWidget(StLabel_);

	QLabel* sloganLabel_ = new QLabel("Get More Views By Multistreaming Directly From OBS", loginWidget);
	sloganLabel_->setStyleSheet("QLabel{font-size: 14px;font-family: Arial;}");
	layout_->addWidget(sloganLabel_);
	sloganLabel_->setWordWrap(true);
	QWidget* scrollWidget = new QWidget;
		// scrollArea->setWidgetResizable(true);
		
		// Create a layout for the widget inside the scroll area
		QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
		layout_->addWidget(scrollWidget);
		scrollWidget->setVisible(false);

		// Add a label and text box for entering the uid
		QLabel* uidLabel_ = new QLabel("Uid", container_);
		layout_->addWidget(uidLabel_);

		QLineEdit* uidLineEdit_ = new QLineEdit(container_);
		layout_->addWidget(uidLineEdit_);

        // Add a label and text box for entering the code
		QLabel* codeLabel_ = new QLabel("API Key", container_);
		layout_->addWidget(codeLabel_);

		QLineEdit* keyLineEdit_ = new QLineEdit(container_);
		layout_->addWidget(keyLineEdit_);

		// Add a button for verification
		QPushButton* verifyButton_ = new QPushButton("Verify", container_);
		layout_->addWidget(verifyButton_);

		QObject::connect(verifyButton_, &QPushButton::clicked, [this, scrollLayout , scrollWidget , uidLineEdit_ , keyLineEdit_ ,verifyButton_ , codeLabel_  , uidLabel_]() {
			// Get the code entered by the user
			QString uid = uidLineEdit_->text();
			QString key = keyLineEdit_->text();

// 		std::promise<CURLcode> promise;
//      std::thread([&]() {
//     // Initialize cURL
//     curl_global_init(CURL_GLOBAL_DEFAULT);
//     CURL *curl = curl_easy_init();
//     curl_slist *list = NULL; 
//     if (curl) {
//         // Set the request URL
//         curl_easy_setopt(curl, CURLOPT_URL, "https://testapi.streamway.in/v1/obs/version");
// 		   list = curl_slist_append(list, "Content-Type: application/json");
// 		   QString combined = uid + ":" + key;
//         // Perform the cURL transfer
//         // CURLcode res = curl_easy_perform(curl);
// 		   QString base64AuthHeader = "Authorization: Basic " + combined.toUtf8().toBase64();
// 		   list = curl_slist_append(list, base64AuthHeader.toStdString().c_str());
// 		   curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

// 					auto WriteCallback = [](char* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
// 					 size_t totalSize = size * nmemb;
// 					 data->append(ptr, totalSize);
// 					 return totalSize;
// 				    };
// 		   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

// 				// Set the response data container
// 				std::string responseData;

// 				// Set the response data pointer
// 				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

				// Perform the request
				// CURLcode res = curl_easy_perform(curl);
        		// Fulfill the promise with the result
        		// promise.set_value(res);

        		// Clean up
//         		curl_easy_cleanup(curl);
//     		}
// 			}).detach();

//       Get a future from the promise
// std::future<CURLcode> future = promise.get_future();

			// Initialize cURL
// 			curl_global_init(CURL_GLOBAL_DEFAULT);
// 			CURL *curl = curl_easy_init();
//          curl_slist *list = NULL; // Define list outside the lambda function
//          if (curl) {
// 				// Set the request URL
// 				curl_easy_setopt(curl, CURLOPT_URL, "https://testapi.streamway.in/v1/obs/version");

// 				// Set the request list
// 				list = curl_slist_append(list, "Content-Type: application/json");
// 				QString combined = uid + ":" + key;
// 				QString base64AuthHeader = "Authorization: Basic " + combined.toUtf8().toBase64();
// 				list = curl_slist_append(list, base64AuthHeader.toStdString().c_str());
// 				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

// 				// Set the response callback function
// 				auto WriteCallback = [](char* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
// 					size_t totalSize = size * nmemb;
// 					data->append(ptr, totalSize);
// 					return totalSize;
// 				};
// 				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

// 				// Set the response data container
// 				std::string responseData;

// 				// Set the response data pointer
// 				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

// 				// Perform the request
// 				CURLcode res = curl_easy_perform(curl);

// 				// Check for errors
// 				if (res != CURLE_OK) {
// 					QLabel *codeLabelcc_ = new QLabel("response data:", container_);
// 					layout_->addWidget(codeLabelcc_);
// 				} else {
// 					// Print the response data
// 					QLabel *codeLabelcc_ = new QLabel("response data:", container_);
// 					layout_->addWidget(codeLabelcc_);
// 				}

// 				// Clean up
// 				curl_slist_free_all(list);
// 				curl_easy_cleanup(curl);
// 			}

			// Clean up cURL
			// curl_global_cleanup();

			QString combined = uid + ":" + key;
			// Construct the request
			QNetworkRequest request(QUrl("http://localhost:8000/v1/obs/version"));
			request.setHeader(QNetworkRequest::ContentTypeHeader,
					  "application/json");

			// Create a QNetworkAccessManager for making the HTTP request
			networkManager_ = new QNetworkAccessManager(this);

			QByteArray combinedData = combined.toUtf8().toBase64();
			QString base64AuthHeader = "Basic " + QString(combinedData);
			request.setRawHeader("Authorization", base64AuthHeader.toUtf8());
			// Send the POST request
			networkReply = networkManager_->get(request);

			// Connect to the finished signal to handle the response
			QObject::connect(networkReply, &QNetworkReply::finished, [this, scrollLayout ,scrollWidget ,  uidLineEdit_ , keyLineEdit_ , verifyButton_ , codeLabel_ ,base64AuthHeader  ,  uidLabel_]() {
				if (networkReply->error() ==
				    QNetworkReply::NoError) {
					// Successful response received
					// obs_data_t *settings = obs_data_create();
					// obs_service_t *service = obs_service_create("custom", "Custom RTMP", settings, NULL);
					
					// Read the response data
					QByteArray responseData =
						networkReply->readAll();

					// TODO: Parse the response data to extract the token
					// Assuming the response contains a JSON object with a "token" field
					jsonDoc = QJsonDocument::fromJson(
						responseData);
					jsonObj = jsonDoc.object();

					if (jsonObj.contains("success")) {
						QString token =
							jsonObj["success"]
								.toString();

						// You have a valid token
						// You can use 'token' as needed
						qDebug() << "Token: " << token;
						handleTab(base64AuthHeader , layout_ , scrollLayout);
						// handleSuccessfulLogin(token ,  scrollLayout);
						// Hide the verification UI
            			 uidLineEdit_->setVisible(false);
    					 keyLineEdit_->setVisible(false);
    					 verifyButton_->setVisible(false);
						 codeLabel_->setVisible(false);
						 scrollWidget->setVisible(true);
						//  StLabel_->setVisible(false);
						 uidLabel_->setVisible(false);
					} else {
						// No token found in the response, indicating an invalid code
						qDebug() << "Invalid token";
						QLabel *codeLabela_ = new QLabel(
							"Invalid token:",
							container_);
						layout_->addWidget(codeLabela_);
					}
				} else {
					// Handle the network error here
					qDebug() << "Network error: "
						 << networkReply->errorString();
					QLabel *codeLabelc_ = new QLabel(
						networkReply->errorString(),
						container_);
					layout_->addWidget(codeLabelc_);
				}

				// Clean up the network resources
				networkReply->deleteLater();
			});
		});

    return loginWidget;
};


// Function to create Tab 1 and its content
QWidget* createTab1(const QString& token) {
    QWidget* tab1 = new QWidget;
    QVBoxLayout* tab1Layout = new QVBoxLayout(tab1);
	tab1->setFixedSize(320, 550);
	handleSuccessfulLogin(token , tab1Layout);
    return tab1;
};

// Function to create Tab 2 and its content
QWidget* createTab2() {
	// QScrollArea* scrollArea = new QScrollArea;
    QWidget* tab2 = new QWidget;
    tab2Layout = new QVBoxLayout(tab2);
	
	// scrollArea->resize(300,300);
	// scrollArea->setWidgetResizable(true);
	// scrollArea->setWidget(tab2);

    // tab2->setFixedSize(320, 600);
	// tab2Layout->addWidget(scrollArea);
	// init widget
		auto addButton = new QPushButton(
			obs_module_text("Btn.NewTarget"), container_);
		QObject::connect(addButton, &QPushButton::clicked, [this]() {
			auto &global = GlobalMultiOutputConfig();
			auto newid = GenerateId(global);
			auto target = std::make_shared<OutputTargetConfig>();
			target->id = newid;
			global.targets.emplace_back(target);

			auto pushwidget = createPushWidget(newid, container_);
			itemLayout_->addWidget(pushwidget);
			

			if (pushwidget->ShowEditDlg())
				SaveConfig();
			else {
				auto it = std::find_if(global.targets.begin(),
						       global.targets.end(),
						       [newid](auto &x) {
							       return x->id ==
								      newid;
						       });
				if (it != global.targets.end())
					global.targets.erase(it);
				delete pushwidget;
			}
		});
		tab2Layout->addWidget(addButton);

		// start all, stop all
		auto allBtnContainer = new QWidget(this);
		auto allBtnLayout = new QHBoxLayout(allBtnContainer);
		auto startAllButton = new QPushButton(
			obs_module_text("Btn.StartAll"), allBtnContainer);
		allBtnLayout->addWidget(startAllButton);
		auto stopAllButton = new QPushButton(
			obs_module_text("Btn.StopAll"), allBtnContainer);
		allBtnLayout->addWidget(stopAllButton);
		allBtnContainer->setLayout(allBtnLayout);
		tab2Layout->addWidget(allBtnContainer);

		QObject::connect(startAllButton, &QPushButton::clicked,
				 [this]() {
					 obs_frontend_streaming_start();
					 for (auto x : GetAllPushWidgets())
						 x->StartStreaming();
				 });
		QObject::connect(stopAllButton, &QPushButton::clicked,
				 [this]() {
					 for (auto x : GetAllPushWidgets())
						 x->StopStreaming();
				 });

		// load config
		itemLayout_ = new QVBoxLayout(tab2);
		LoadConfig();
		tab2Layout->addLayout(itemLayout_);

    return tab2;
};


// Function to create Tab 1 and its content
QWidget* createTab3() {
QWidget* tab3 = new QWidget;
QVBoxLayout* tab3Layout = new QVBoxLayout(tab3);
QString quote = "Credit -> obs-multi-rtmp plugin by sorayuki\n"
				"This plugin is an extention of obs-multi-rtmp and it intents to empower users to multistream from within OBS itself. "
				"Purpose of plugin is to let user schedule event from within the OBS studio itself rathar than opening multiple tabs of diffrent social platforms like youtube and facebook and manually copy pasting the keys etc. "
				"https://github.com/sorayuki/obs-multi-rtmp";

auto linkLable = new QLabel(
				u8"<p> <a href=\"https://github.com/sorayuki/obs-multi-rtmp\"></a> </p>",
				tab3);
			linkLable->setTextFormat(Qt::RichText);
			linkLable->setTextInteractionFlags(
				Qt::TextBrowserInteraction);
			linkLable->setOpenExternalLinks(true);				

QLabel* quoteLabel = new QLabel(quote);
quoteLabel->setWordWrap(true);
tab3Layout->addWidget(quoteLabel);
tab3Layout->addWidget(linkLable);
// Add the quote label to the layout or widget where you want to display it
tab3Layout->addWidget(quoteLabel);
    return tab3;
};

void handleTab(const QString& token , QVBoxLayout *layout , QVBoxLayout *newUiLayout) {
	// Create a QTabWidget to hold the tabs
	QTabWidget* tabWidget = new QTabWidget;

	// Call the functions to create tabs and add them to the QTabWidget
    tabWidget->addTab(createTab1(token), "Broadcasts");
    tabWidget->addTab(createTab2(), "Go Live");
	tabWidget->addTab(createTab3(), "About");
	newUiLayout->addWidget(tabWidget);
};

void handleSuccessfulLogin(const QString& token , QVBoxLayout *newUiLayout) {

    QNetworkRequest request(QUrl("http://localhost:8000/v1/broadcasts/upcoming"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    request.setRawHeader("Authorization", token.toUtf8());
   

    QNetworkAccessManager networkManager;
    QNetworkReply* networkReply = networkManager.get(request);

    // Create an event loop to wait for the network request to finish
    QEventLoop eventLoop;
    QObject::connect(&networkManager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);

    eventLoop.exec(); // This will block until the network request finishes

    if (networkReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = networkReply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonArray jsonArray = jsonDoc.array();

		

        QLabel* tokenLabelSize = new QLabel("Total broadcasts " + QString::number(jsonArray.size()));
        newUiLayout->addWidget(tokenLabelSize);


		// Create a scroll area and set up a widget to contain the items
QScrollArea* scrollArea = new QScrollArea;
QWidget* scrollWidget = new QWidget;

// scrollWidget->resize(300,300);
scrollArea->setWidgetResizable(true);
scrollArea->setWidget(scrollWidget);

// Create a layout for the widget inside the scroll area
QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

for (const QJsonValue& jsonValue : jsonArray) {
    if (jsonValue.isObject()) {
        QJsonObject jsonObject = jsonValue.toObject();
        if (jsonObject.contains("title")) {
            // Create a custom widget to represent each item
            QWidget* itemWidget = new QWidget;
            QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);

			// Create a group for title and scheduledTime
            QGroupBox* titleScheduledGroup = new QGroupBox;
            QVBoxLayout* titleScheduledLayout = new QVBoxLayout(titleScheduledGroup);

			// Set a fixed size for the QGroupBox
			titleScheduledGroup->setMinimumSize(220, 110); // Set the minimum width and height
			// Set padding or margins for the contents of the QGroupBox
			int leftMargin = 10; // Adjust as needed
			int topMargin = 0;  // Adjust as needed
			int rightMargin = 10; // Adjust as needed
			int bottomMargin = 10;  // Adjust as needed
			titleScheduledLayout->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);

			// Retrieve the image URL from the jsonObject
			QString imageUrl = jsonObject["thumbnail"].toString();
            // Show jsonObject thumbnail field in an image (Assuming you have a QLabel for this)
            QLabel* thumbnailLabel = new QLabel;
            
			
			thumbnailLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			QNetworkAccessManager networkManager;
			// Create a QUrl object with the image URL
			QUrl url(imageUrl);

			QNetworkRequest request(url);
			QNetworkReply* reply = networkManager.get(request);

			// Handle the image loading asynchronously
			QObject::connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        QImage thumbnail;
        thumbnail.loadFromData(imageData); // Load image data into the QImage instance
        QPixmap pixmap = QPixmap::fromImage(thumbnail);
        thumbnailLabel->setPixmap(pixmap);
    } else {
        qDebug() << "Failed to load image:" << reply->errorString();
    }
    reply->deleteLater();
});


			
			titleScheduledLayout->addWidget(thumbnailLabel); // Add the image label
            // Show title from jsonObject
            QString title = jsonObject["title"].toString();
            QLabel* titleLabel = new QLabel(title);
			titleLabel->setWordWrap(true);
			titleLabel->setStyleSheet("QLabel{font-size: 15px;font-family: Arial;}"); 
            titleScheduledLayout->addWidget(titleLabel);


            // Show scheduledTime from jsonObject
            QString scheduledTimeStr = jsonObject["scheduledTime"].toString();
			// scheduledTimeStr.resize(10);

			// QString scheduledTimeStr = "2023-11-06T16:24";

// Parse the input string
QDateTime scheduledTime = QDateTime::fromString(scheduledTimeStr, "yyyy-MM-ddTHH:mm");

// Format the datetime as per your requirement
QString formattedTime = scheduledTime.toString("dddd, MMMM d 'at' h:mm AP");
			// QLabel* gfgf = new QLabel("scheduledTime");
            // titleScheduledLayout->addWidget(gfgf);

            QLabel* timeLabel = new QLabel(formattedTime);
            // titleScheduledLayout->addWidget(timeLabel);
				timeLabel->setStyleSheet("QLabel{font-size: 12px;font-family: Arial;}");
				titleScheduledLayout->addWidget(timeLabel);
			QPushButton* SelectButton = new QPushButton("Select");
			QObject::connect(SelectButton, &QPushButton::clicked, [this , jsonObject]() {
	
				

                	auto &global = GlobalMultiOutputConfig();
				
                	global.targets.clear();
					SaveMultiOutputConfig();
					LoadConfig();
					tab2Layout = 0;

			 QJsonArray destinationsArray = jsonObject["destinations"].toArray();
			 QJsonObject firstDestination = destinationsArray[0].toObject();
			 	obs_service_t *service = obs_frontend_get_streaming_service();
    			obs_data_t *settings = obs_service_get_settings(service);
    			// cout << obs_data_get_json_pretty(settings) << endl;
				QString url = firstDestination["url"].toString();
				QString key = firstDestination["key"].toString();
				obs_data_set_string(settings, "key", key.toStdString().c_str());
				obs_data_set_string(settings, "server", url.toStdString().c_str());
    			obs_data_release(settings);

				
					for (int i = 1; i < destinationsArray.size(); i++) {
        				QJsonObject destination = destinationsArray[i].toObject();
        				QString platformUserName = destination["platformUserName"].toString();
        				QString platformTitle = destination["platformTitle"].toString();
        				QString title = platformUserName + " / " + platformTitle;
        				auto newid = GenerateId(global);
        				auto target = std::make_shared<OutputTargetConfig>();
        				target->id = newid;
        				target->name = title.toStdString();
       				 	target->serviceParam = {
            				{"server", destination["url"].toString().toStdString()},
            				{"key", destination["key"].toString().toStdString()}
        				};
						target->syncStart = true;
        				global.targets.emplace_back(target);

        				auto pushwidget = createPushWidget(newid, container_);
        				itemLayout_->addWidget(pushwidget);
        				SaveConfig();
    				}
				
    		 
        // Create and add a QLabel for each destination
        // QLabel* destinationLabel = new QLabel(QString("Destination: %1").arg(target->name));
        // titleScheduledLayout->addWidget(destinationLabel);
    	});
			titleScheduledLayout->addWidget(SelectButton);

			// Add the title and scheduledTime group to the item layout
            itemLayout->addWidget(titleScheduledGroup);



            // Add the custom widget to the scroll layout
            scrollLayout->addWidget(itemWidget);
        }
    }
}


// Create a horizontal layout for Add and Cancel buttons
QHBoxLayout* buttonLayout = new QHBoxLayout;

// Create Cancel and Add buttons outside of the loop
// QPushButton* cancelButton = new QPushButton("Cancel");
QPushButton* addButton = new QPushButton("Schedule New Broadcast");

// Add the scroll area to your main layout or window
newUiLayout->addWidget(scrollArea);
// Add Cancel and Add buttons to the layout or wherever appropriate
// buttonLayout->addWidget(cancelButton);
buttonLayout->addWidget(addButton);
QObject::connect(addButton, &QPushButton::clicked,
				 [this]() {
					 // URL to open in the default web browser
        // QUrl url("https://app.streamway.in");

        // Open the URL in the user's default web browser
        QDesktopServices::openUrl(QUrl("https://app.streamway.in/"));
				 });
// Create a container widget for the button layout
QWidget* buttonContainer = new QWidget;
buttonContainer->setLayout(buttonLayout);
newUiLayout->addWidget(buttonContainer);
// Connect signals and slots for Cancel and Add buttons
// QObject::connect(cancelButton, &QPushButton::clicked, window, &QWidget::close);
        
    } else {
        qDebug() << "Network error: " << networkReply->errorString();
        // Handle network errors here
    }

    networkReply->deleteLater();

    // newUiLayout->addWidget(successLabel);
  
	
    // newUi->setCentralWidget(centralWidget);
    // newUi->show();
}



	void visibleToggled(bool visible)
	{
		dockVisible_ = visible;

		if (visible == false && reopenShown_ == false &&
		    !config_has_user_value(obs_frontend_get_global_config(),
					   ConfigSection, "DockVisible")) {
			reopenShown_ = true;
			QMessageBox(QMessageBox::Icon::Information,
				    obs_module_text("Notice.Title"),
				    obs_module_text("Notice.Reopen"),
				    QMessageBox::StandardButton::Ok, this)
				.exec();
		}
	}

	bool event(QEvent *event) override
	{
		if (event->type() == QEvent::Resize) {
			scroll_->resize(width(), height() - 22);
		}
		return QDockWidget::event(event);
	}

	std::vector<PushWidget *> GetAllPushWidgets()
	{
		std::vector<PushWidget *> result;
		for (auto &c : container_->children()) {
			if (c->objectName() == "push-widget") {
				auto w = dynamic_cast<PushWidget *>(c);
				result.push_back(w);
			}
		}
		return result;
	}

	void SaveConfig() { SaveMultiOutputConfig(); }




void LoadConfig() {
	 // Clear previous pushwidgets from itemLayout_
    while (QLayoutItem *item = itemLayout_->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->hide(); // Optional: Hide the widget before deleting
            delete widget;
        }
        delete item;
    }
	
    for (auto x : GetAllPushWidgets()) {
        delete x;
    }
    GlobalMultiOutputConfig() = {};

    if (LoadMultiOutputConfig() == false) {
        ImportLegacyMultiOutputConfig();
    }
	
    for (auto x : GlobalMultiOutputConfig().targets) {
        auto pushwidget = createPushWidget(x->id, container_);
        itemLayout_->addWidget(pushwidget);
	}
}


private:
	QWidget *container_ = 0;
	QScrollArea *scroll_ = 0;
	QVBoxLayout *itemLayout_ = 0;
	QVBoxLayout *layout_ = 0;
	QVBoxLayout* tab2Layout = 0;
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-multi-rtmp", "en-US")
OBS_MODULE_AUTHOR("雷鳴 (@sorayukinoyume)")

bool obs_module_load()
{
	// check obs version
	if (obs_get_version() < MAKE_SEMANTIC_VERSION(29, 1, 0)) {
		blog(LOG_ERROR, TAG "Request OBS 29.1 or higher.");
		return false;
	}

	auto mainwin = (QMainWindow *)obs_frontend_get_main_window();
	if (mainwin == nullptr)
		return false;
	QMetaObject::invokeMethod(mainwin, []() {
		s_service.uiThread_ = QThread::currentThread();
	});

	auto dock = new MultiOutputWidget(mainwin);
	dock->setObjectName("obs-multi-rtmp-dock");
	auto action = (QAction *)obs_frontend_add_dock(dock);
	QObject::connect(action, &QAction::toggled, dock,
			 &MultiOutputWidget::visibleToggled);

	obs_frontend_add_event_callback(
		[](enum obs_frontend_event event, void *private_data) {
			auto mainwin =
				static_cast<MultiOutputWidget *>(private_data);

			for (auto x : mainwin->GetAllPushWidgets())
				x->OnOBSEvent(event);

			if (event ==
			    obs_frontend_event::OBS_FRONTEND_EVENT_EXIT) {
				mainwin->SaveConfig();
			} else if (event ==
				   obs_frontend_event::
					   OBS_FRONTEND_EVENT_PROFILE_CHANGED) {
				mainwin->LoadConfig();
			}
		},
		dock);

	return true;
}

const char *obs_module_description(void)
{
	return "Multiple RTMP Output Plugin";
}


// import obspython as obs
 
// key = ''
 
// def script_properties():
//     props = obs.obs_properties_create()
//     obs.obs_properties_add_text(props, "key", "Stream Key", obs.OBS_TEXT_DEFAULT)
//     obs.obs_properties_add_button(props, "button", "Set", callback)
//     return props
 
// def script_update(settings):
//     global key
//     key = obs.obs_data_get_string(settings, 'key')
       
// def callback(props, prop):
//     global key
//     service = obs.obs_frontend_get_streaming_service()
//     settings = obs.obs_service_get_settings(service)
//     print(obs.obs_data_get_json_pretty(settings)) # so you can see what it actually looks like
//     obs.obs_data_set_string(settings, 'key', key)
//     obs.obs_data_release(settings)
//     return False # Can also be true, doesn't matter