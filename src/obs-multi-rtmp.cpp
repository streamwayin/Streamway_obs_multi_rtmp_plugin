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

using Json = nlohmann::json;

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
	std::string currentBroadcast;

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
	CURL *curl;
  	CURLcode res;

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


		 QString uid, key;

    auto profiledir = obs_frontend_get_current_profile_path();
    if (profiledir) {
        QString filename = QString::fromStdString(std::string(profiledir) + "/obs-multi-rtmp_auth.json");

        QFile file(filename);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray jsonData = file.readAll();
            QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData));

            if (!jsonDoc.isNull() && jsonDoc.isObject()) {
                QJsonObject jsonObject = jsonDoc.object();

                if (jsonObject.contains("uid_key") && jsonObject["uid_key"].isObject()) {
                    QJsonObject uidKeyObj = jsonObject["uid_key"].toObject();
                    if (uidKeyObj.contains("uid") && uidKeyObj.contains("key")) {
                        uid = uidKeyObj["uid"].toString();
                        key = uidKeyObj["key"].toString();
                    }
                }
            }
            file.close();
        }
        bfree(profiledir);
    }

if (!uid.isEmpty() && !key.isEmpty()) {
	 container_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    container_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    container_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    handleTab(layout_ , layout_, uid , key);
} else {
   handleAuthTab(layout_);
}
	

		
		



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
QWidget* LoginWithPhoneWidget() {
    QWidget* loginWithPhoneWidget = new QWidget;
QVBoxLayout* LoginLayout = new QVBoxLayout(loginWithPhoneWidget);

QLabel* sloganLabel_ = new QLabel("Get More Views By Multistreaming Directly From OBS", loginWithPhoneWidget);
sloganLabel_->setStyleSheet("QLabel{font-size: 14px;font-family: Arial;}");
LoginLayout->addWidget(sloganLabel_);
sloganLabel_->setWordWrap(true);

QWidget* scrollWidget = new QWidget;
		// scrollArea->setWidgetResizable(true);
		
		// Create a layout for the widget inside the scroll area
		QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
		LoginLayout->addWidget(scrollWidget);
		scrollWidget->setVisible(false);

// Mobile Number text box
QLabel* phone_ = new QLabel("Enter Phone Number", container_);
LoginLayout->addWidget(phone_);

auto horizontalPhoneLayout = new QHBoxLayout;
QLineEdit* prePhoneLineEdit_ = new QLineEdit(container_);
prePhoneLineEdit_->setText("+91");
prePhoneLineEdit_->setMaximumWidth(40);
horizontalPhoneLayout->addWidget(prePhoneLineEdit_);

QLineEdit* phoneLineEdit_ = new QLineEdit(container_);
phoneLineEdit_->setValidator(new QIntValidator(0, 1000000000, this));
horizontalPhoneLayout->addWidget(phoneLineEdit_);

LoginLayout->addLayout(horizontalPhoneLayout);

// Add a button for getting OTP
QPushButton* getOTPButton_ = new QPushButton("Get OTP", container_);
LoginLayout->addWidget(getOTPButton_);

// OTP input and Verify button
QLineEdit* otpLineEdit_ = new QLineEdit(container_);
otpLineEdit_->setValidator(new QIntValidator(0, 1000, this));
otpLineEdit_->setPlaceholderText("Enter OTP here");

QPushButton* verifyOTPButton_ = new QPushButton("Verify", container_);

LoginLayout->addWidget(otpLineEdit_);
LoginLayout->addWidget(verifyOTPButton_);
verifyOTPButton_->setVisible(false);
otpLineEdit_->setReadOnly(true);

QLabel* errorL = new QLabel();
LoginLayout->addWidget(errorL);
errorL->setWordWrap(true);
errorL->setStyleSheet("QLabel{font-size: 15px;font-family: Arial;color:red;}");
errorL->setVisible(false);



		QObject::connect(getOTPButton_ , &QPushButton::clicked, [this, scrollLayout , scrollWidget  ,phone_ , phoneLineEdit_ , getOTPButton_ , prePhoneLineEdit_ , otpLineEdit_ , verifyOTPButton_  ,errorL](){
			QString fullPhoneNumber_ = prePhoneLineEdit_->text() + phoneLineEdit_->text();
			
			phoneLineEdit_->setReadOnly(true);
			prePhoneLineEdit_->setReadOnly(true);
			getOTPButton_->setEnabled(false);
			
			
			Json jsonObject;
			jsonObject["phoneNumber"] = fullPhoneNumber_.toStdString();
			   // Convert JSON object to string
    			std::string jsonData = jsonObject.dump(2);

			CURL *curl;
    		CURLcode res;
			curl = curl_easy_init();

			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/v1/otp/obs/phone/send");
			// curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
			// curl_easy_setopt(curl , CURLOPT_WRITEFUNCTION , writeCallback);

			struct curl_slist *headers = NULL;
			headers = curl_slist_append(headers, "Accept: */*");
			headers = curl_slist_append(headers, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

			CURLcode ret = curl_easy_perform(curl);

			long response_code;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

	    		if (response_code != 200) {
        			curl_easy_cleanup(curl);
					
        			errorL->setText("This Phone Number is not Registerd on Streamway");
    			}else{
					verifyOTPButton_->setVisible(true);
					errorL->setVisible(false);
					getOTPButton_->setVisible(false);
					verifyOTPButton_->setVisible(true);
					otpLineEdit_->setVisible(true);
					otpLineEdit_->setReadOnly(false);
				}

	  curl_easy_cleanup(curl);
		curl = NULL;
			
		});

		QObject::connect(verifyOTPButton_ , &QPushButton::clicked, [this,scrollLayout , scrollWidget  ,phone_ , phoneLineEdit_ , getOTPButton_ , prePhoneLineEdit_ , otpLineEdit_ , verifyOTPButton_ , errorL ](){
			phone_->setText("Enter OTP");
			phoneLineEdit_->setReadOnly(true);
			prePhoneLineEdit_->setReadOnly(true);
			getOTPButton_->setEnabled(false);
			verifyOTPButton_->setEnabled(false);
			QString fullPhoneNumber_ = prePhoneLineEdit_->text() + phoneLineEdit_->text();
			QString otp = otpLineEdit_->text();
			bool ok;
			int otpAsNumber = otp.toInt(&ok);
			Json jsonObject;
			jsonObject["phoneNumber"] = fullPhoneNumber_.toStdString();
			jsonObject["otp"] = otpAsNumber;
			   // Convert JSON object to string
    			std::string jsonData = jsonObject.dump(2);

			CURL *curl;
    		CURLcode res;
			std::string readBuffer;
			curl = curl_easy_init();

			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/v1/otp/obs/verify");
			// curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
			// curl_easy_setopt(curl , CURLOPT_WRITEFUNCTION , writeCallback);

			struct curl_slist *headers = NULL;
			headers = curl_slist_append(headers, "Accept: */*");
			headers = curl_slist_append(headers, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
			curl_easy_setopt(curl , CURLOPT_WRITEFUNCTION , writeCallback);
			curl_easy_setopt(curl , CURLOPT_WRITEDATA , &readBuffer);

			CURLcode ret = curl_easy_perform(curl);

			long response_code;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

	    			if (response_code != 200) {
        			curl_easy_cleanup(curl);
        			phone_->setVisible(false);
					phone_->setVisible(false);
					phoneLineEdit_->setVisible(false);
					getOTPButton_->setVisible(false);
					prePhoneLineEdit_->setVisible(false);
					verifyOTPButton_->setVisible(false);
					otpLineEdit_->setVisible(false);

    			}else if(response_code == 400 || response_code == 404){
					auto j3 = Json::parse(readBuffer);
					Json object = j3;
					std::string error = j3["msg"];
					errorL->setVisible(true);
					errorL->setText(QString::fromStdString(error));
				}else{
					
					errorL->setText("Internal Error Please Contact Us");
				}

	  		curl_easy_cleanup(curl);
			curl = NULL;

			

		});

		

    return loginWithPhoneWidget;
};


QWidget* LoginWithEmailWidget() {
	 QWidget* loginWithEmailWidget = new QWidget;
    QVBoxLayout* LoginLayout = new QVBoxLayout(loginWithEmailWidget);

	QLabel* sloganLabel_ = new QLabel("Get More Views By Multistreaming Directly From OBS", loginWithEmailWidget);
	sloganLabel_->setStyleSheet("QLabel{font-size: 14px;font-family: Arial;}");
	LoginLayout->addWidget(sloganLabel_);
	sloganLabel_->setWordWrap(true);
	QWidget* scrollWidget = new QWidget;
		// scrollArea->setWidgetResizable(true);
		
		// Create a layout for the widget inside the scroll area
		QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
		LoginLayout->addWidget(scrollWidget);
		scrollWidget->setVisible(false);

		// Add a label and text box for entering the uid
		QLabel* emailLabel_ = new QLabel("Email", container_);
		LoginLayout->addWidget(emailLabel_);

		QLineEdit* emailLineEdit_ = new QLineEdit(container_);
		LoginLayout->addWidget(emailLineEdit_);

        // Add a label and text box for entering the code
		QLabel* codeLabel_ = new QLabel("API Key", container_);
		LoginLayout->addWidget(codeLabel_);

		QLineEdit* keyLineEdit_ = new QLineEdit(container_);
		LoginLayout->addWidget(keyLineEdit_);

	
		// Add a button for verification
		QPushButton* verifyButton_ = new QPushButton("Verify", container_);
		LoginLayout->addWidget(verifyButton_);

		QLabel* errorL = new QLabel("Invalid Uid or Api Key");
		errorL->setStyleSheet("QLabel{font-size: 14px;font-family: Arial;}");
		scrollLayout->addWidget(errorL);
		errorL->setWordWrap(true);
		errorL->setStyleSheet("QLabel{font-size: 15px;font-family: Arial;color:red;}"); 
		errorL->setVisible(false);

		

		QObject::connect(verifyButton_, &QPushButton::clicked, [this,errorL,scrollLayout , scrollWidget , emailLineEdit_ , keyLineEdit_ ,verifyButton_ , codeLabel_  , emailLabel_ ]() {
			// Get the code entered by the user
			QString uid = emailLineEdit_->text();
			QString key = keyLineEdit_->text();

			QString combined = uid + ":" + key;
		

			QByteArray combinedData = combined.toUtf8().toBase64();
			QString base64AuthHeader = "Basic " + QString(combinedData);

  std::string url = "https://api.example.com/endpoint";
  std::string authHeader = "Authorization: Bearer YOUR_API_KEY";

  std::string response;
 
 
  

   // Use a try-catch block to catch any potential exceptions
    try {
       // Send the HTTP request
  if(sendHttpRequest(url, authHeader, response , uid , key)){
	try{

				// // Hide the verification UI
            			 emailLineEdit_->setVisible(false);
    					 keyLineEdit_->setVisible(false);
    					 verifyButton_->setVisible(false);
						 codeLabel_->setVisible(false);
						 scrollWidget->setVisible(true);
						 emailLabel_->setVisible(false);
						 	errorL->setVisible(false);
					handleTab( layout_ , scrollLayout, uid , key);
						
	}catch (const std::exception& e) {
		scrollWidget->setVisible(true);
       	errorL->setVisible(true);
		
	}
						
  }else{
	scrollWidget->setVisible(true);
	errorL->setVisible(true);
  }
    } catch (const std::exception& e) {
		scrollWidget->setVisible(true);
        errorL->setVisible(true);
    }			
		});

    return loginWithEmailWidget;
}


QWidget* LoginWithAPIKeyWidget(QTabWidget *tabWidget) {
	 QWidget* loginWithAPIKeyWidget = new QWidget;
    QVBoxLayout* LoginLayout = new QVBoxLayout(loginWithAPIKeyWidget);

	QLabel* sloganLabel_ = new QLabel("Get More Views By Multistreaming Directly From OBS", loginWithAPIKeyWidget);
	sloganLabel_->setStyleSheet("QLabel{font-size: 14px;font-family: Arial;}");
	LoginLayout->addWidget(sloganLabel_);
	sloganLabel_->setWordWrap(true);
	QWidget* scrollWidget = new QWidget;
		// scrollArea->setWidgetResizable(true);
		
		// Create a layout for the widget inside the scroll area
		QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);
		LoginLayout->addWidget(scrollWidget);
		scrollWidget->setVisible(false);

		// Add a label and text box for entering the uid
		QLabel* uidLabel_ = new QLabel("Uid", container_);
		LoginLayout->addWidget(uidLabel_);

		QLineEdit* uidLineEdit_ = new QLineEdit(container_);
		LoginLayout->addWidget(uidLineEdit_);

        // Add a label and text box for entering the code
		QLabel* codeLabel_ = new QLabel("API Key", container_);
		LoginLayout->addWidget(codeLabel_);

		QLineEdit* keyLineEdit_ = new QLineEdit(container_);
		LoginLayout->addWidget(keyLineEdit_);

	
		// Add a button for verification
		QPushButton* verifyButton_ = new QPushButton("Verify", container_);
		LoginLayout->addWidget(verifyButton_);

		QLabel* errorL = new QLabel("Invalid Uid or Api Key");
		errorL->setStyleSheet("QLabel{font-size: 14px;font-family: Arial;}");
		scrollLayout->addWidget(errorL);
		errorL->setWordWrap(true);
		errorL->setStyleSheet("QLabel{font-size: 15px;font-family: Arial;color:red;}"); 
		errorL->setVisible(false);


		QObject::connect(verifyButton_, &QPushButton::clicked, [this,errorL,scrollLayout , scrollWidget , uidLineEdit_ , keyLineEdit_ ,verifyButton_ , codeLabel_  , uidLabel_ ,tabWidget]() {
			// Get the code entered by the user
			QString uid = uidLineEdit_->text();
			QString key = keyLineEdit_->text();

			QString combined = uid + ":" + key;
		

			QByteArray combinedData = combined.toUtf8().toBase64();
			QString base64AuthHeader = "Basic " + QString(combinedData);

  std::string url = "https://api.example.com/endpoint";
  std::string authHeader = "Authorization: Bearer YOUR_API_KEY";

  std::string response;
 

   // Use a try-catch block to catch any potential exceptions
    try {
       // Send the HTTP request
  if(sendHttpRequest(url, authHeader, response , uid , key)){
	try{

	 auto profiledir = obs_frontend_get_current_profile_path();
        if (profiledir) {
            std::string filename = profiledir;
            filename += "/obs-multi-rtmp_auth.json";

            // Read existing JSON content from the file
            std::ifstream inFile(filename);
            nlohmann::json configJson;

            if (inFile.is_open()) {
                inFile >> configJson;
                inFile.close();
            } 

            // Update uid and key in the existing JSON object
            configJson["uid_key"]["uid"] = uid.toStdString();
            configJson["uid_key"]["key"] =  key.toStdString();
			 

            // Convert the updated JSON to a string
            std::string content = configJson.dump();

			

            // Write the updated content back to the file
            os_quick_write_utf8_file_safe(filename.c_str(), content.c_str(), content.size(), true, "tmp", "bak");
        }
        bfree(profiledir);

				// // Hide the verification UI
            			 uidLineEdit_->setVisible(false);
    					 keyLineEdit_->setVisible(false);
    					 verifyButton_->setVisible(false);
						 codeLabel_->setVisible(false);
						 scrollWidget->setVisible(true);
						 uidLabel_->setVisible(false);
						 	errorL->setVisible(false);


							
					handleTab( layout_ , scrollLayout, uid , key);
						// while (tabWidget->count() > 0) {
            			// 	QWidget* tabToRemove = tabWidget->widget(0); // Get the first 		tab
            			// 	tabWidget->removeTab(0); // Remove the first tab
            			// 	delete tabToRemove; // Optionally delete the removed tab widget
       					//  }
	}catch (const std::exception& e) {
		scrollWidget->setVisible(true);
       	errorL->setVisible(true);
		
	}
						
  }else{
	scrollWidget->setVisible(true);
	errorL->setVisible(true);
  }
    } catch (const std::exception& e) {
		scrollWidget->setVisible(true);
        errorL->setVisible(true);
    }			
		});

    return loginWithAPIKeyWidget;
}

// Function to handle Authentation Tabs
void handleAuthTab( QVBoxLayout *layout) {
	// Create a QTabWidget to hold the tabs
	QTabWidget* tabWidget = new QTabWidget;

	// Call the functions to create tabs and add them to the QTabWidget
  // Create widgets for each tab content
    QWidget* phoneWidget = LoginWithPhoneWidget();
    QWidget* emailWidget = LoginWithEmailWidget();
    QWidget* apiKeyWidget = LoginWithAPIKeyWidget(tabWidget);

    // Add tabs to the QTabWidget
    // tabWidget->addTab(phoneWidget, "Phone");
    // tabWidget->addTab(emailWidget, "Email");
    tabWidget->addTab(apiKeyWidget, "API Key");
	layout->addWidget(tabWidget);
};

// Define the callback function to capture response data

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *context) {
  size_t bytec = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)context;
  void *temp = realloc(mem->memory, mem->size + bytec + 1);
   if (temp != NULL) {
       mem->memory = (char *)temp;
        mem->size += bytec;
    }
  if(mem->memory == NULL) {
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
  memcpy(&(mem->memory[mem->size]), ptr, bytec);
  mem->size += bytec;
  mem->memory[mem->size] = 0;
  return nmemb;
}

// Function to send an HTTP request and parse JSON response
bool sendHttpRequest(std::string &url, std::string &authHeader, std::string &response, const QString& uid, const QString& key) {
    CURL *curl;
    CURLcode res;

    // Initialize curl
    curl = curl_easy_init();
    if (!curl) {
        return false;
    };

    // Set the URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://testapi.streamway.in/v1/obs/version");

    // Set the request method to GET
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    // Set authentication
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERNAME, uid.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, key.toStdString().c_str());

    // Set headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request
    res = curl_easy_perform(curl);

	long response_code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    // Check for errors
    if (response_code != 200) {
        curl_easy_cleanup(curl);
        return false;
    }

	if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return false;
    }

    // Cleanup
    curl_easy_cleanup(curl);
	curl = NULL;

    return true;
}

// Function to create Tab 1 and its content
QWidget* createTab1(const QString& uid, const QString& key , QTabWidget *tabWidget) {
    QWidget* tab1 = new QWidget;
    QVBoxLayout* tab1Layout = new QVBoxLayout(tab1);
	tab1->setFixedSize(320, 550);
	handleSuccessfulLogin(uid , key , tab1Layout , tabWidget);
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
		auto endAllBroadcastButton = new QPushButton("End Broadcast");
		tab2Layout->addWidget(endAllBroadcastButton);

		QObject::connect(endAllBroadcastButton, &QPushButton::clicked, [this]() {
			
			CURL *curl;
    		CURLcode res;
			curl = curl_easy_init();

			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			std::string url = "http://localhost:8000/v1/broadcasts/" + currentBroadcast;
			curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
			// curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
			// curl_easy_setopt(curl , CURLOPT_WRITEFUNCTION , writeCallback);

			struct curl_slist *headers = NULL;
			headers = curl_slist_append(headers, "Accept: */*");
			headers = curl_slist_append(headers, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			

			CURLcode ret = curl_easy_perform(curl);
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    		// Check for errors
    		if (response_code != 200) {
    		    curl_easy_cleanup(curl);
    		    return false;
    		}

			// if (res != CURLE_OK) {
    		//     curl_easy_cleanup(curl);
    		//     return false;
    		// }

    		// Cleanup
    		curl_easy_cleanup(curl);
			curl = NULL;

		})


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



// Function to recreate the UI layout
void recreateUILayout(QVBoxLayout* newUiLayout) {
    // Clear the existing layout
    QLayoutItem* item;
    while ((item = newUiLayout->takeAt(0)) != nullptr) {
        delete item->widget(); // Remove and delete widgets in the layout
        delete item; // Delete layout items
    }

    // Recreate the layout
    handleAuthTab(newUiLayout);// Call your handleTab function or create a new layout here
}

// Function to create Tab 1 and its content
QWidget* createTab3(QVBoxLayout* newUiLayout) {
QWidget* tab3 = new QWidget;

QVBoxLayout* tab3Layout = new QVBoxLayout(tab3);

		auto logOutButton = new QPushButton("LogOut" , tab3);
		tab3Layout->addWidget(logOutButton);
		QObject::connect(logOutButton, &QPushButton::clicked,
				 [this , newUiLayout]() {
					auto profiledir = obs_frontend_get_current_profile_path();
        if (profiledir) {
            std::string filename = profiledir;
            filename += "/obs-multi-rtmp_auth.json";

            // Read existing JSON content from the file
            std::ifstream inFile(filename);
            nlohmann::json configJson;

            if (inFile.is_open()) {
                inFile >> configJson;
                inFile.close();
            } 

            // Update uid and key in the existing JSON object
            configJson["uid_key"]["uid"] ="";
            configJson["uid_key"]["key"] =  "";
			 

            // Convert the updated JSON to a string
            std::string content = configJson.dump();

			

            // Write the updated content back to the file
            os_quick_write_utf8_file_safe(filename.c_str(), content.c_str(), content.size(), true, "tmp", "bak");
        }
        bfree(profiledir);

		recreateUILayout(newUiLayout);

 });

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
tab3Layout->addWidget(quoteLabel);
quoteLabel->setWordWrap(true);
tab3Layout->addWidget(linkLable);
// Add the quote label to the layout or widget where you want to display it
tab3Layout->addWidget(quoteLabel);
    return tab3;
};

void handleTab( QVBoxLayout *layout , QVBoxLayout *newUiLayout ,const QString& uid, const QString& key) {
	// Create a QTabWidget to hold the tabs
	QTabWidget* tabWidget = new QTabWidget;
	QWidget* tab1 = createTab1(uid, key , tabWidget);
    QWidget* tab2 = createTab2();
    QWidget* tab3 = createTab3(newUiLayout);

    // Set size policies to prevent unnecessary space
    tab1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    tab2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    tab3->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	 // Set fixed sizes for tabs
    // tab1->setFixedSize(200, 50); // Set width and height for tab1
    tab2->setMinimumSize(300, 200); // Set minimum width and height for tab2
    tab3->setMaximumSize(240, 230); // Set maximum width and height for tab3

	 // Set background color for each tab
    // tab1->setStyleSheet("background-color: lightblue;"); // Change color as needed
    // tab2->setStyleSheet("background-color: lightgreen;"); // Change color as needed
    // tab3->setStyleSheet("background-color: lightyellow;"); // Change color as needed

    // Add tabs to the QTabWidget
    tabWidget->addTab(tab1, "Broadcasts");
    tabWidget->addTab(tab2, "Go Live");
    tabWidget->addTab(tab3, "About");
	newUiLayout->addWidget(tabWidget);
};


static size_t writeCallback(void *ptr, size_t size, size_t nmemb, void *stream) {
  std::string *data = (std::string *)stream;
  data->append((char *)ptr, size * nmemb);
  return size * nmemb;
};
 

void handleSuccessfulLogin(const QString& uid, const QString& key, QVBoxLayout *newUiLayout , QTabWidget *tabWidget) {


	CURL *curl;
    CURLcode res;
	std::string readBuffer;
    // Initialize curl
    curl = curl_easy_init();
    // Set the URL
    curl_easy_setopt(curl, CURLOPT_URL, "https://testapi.streamway.in/v1/broadcasts/upcoming");

    // Set the request method to GET
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    // Set authentication
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERNAME, uid.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, key.toStdString().c_str());
    curl_easy_setopt(curl , CURLOPT_WRITEFUNCTION , writeCallback);
	curl_easy_setopt(curl , CURLOPT_WRITEDATA , &readBuffer);
    // Set headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request
    res = curl_easy_perform(curl);

	long response_code;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    // Check for errors
    // if (response_code != 200) {
    //     curl_easy_cleanup(curl);
    //     return false;
    // }

	// if (res != CURLE_OK) {
    //     curl_easy_cleanup(curl);
    //     return false;
    // }

    // Cleanup
    curl_easy_cleanup(curl);
	curl = NULL;

	auto j3 = Json::parse(readBuffer);
	Json object = j3;
	std::string title =j3[0]["title"];
	// QString qTitle = QString::fromStdString(title);
	// QLabel* tokenLabelSizep = new QLabel(qTitle);
    //     newUiLayout->addWidget(tokenLabelSizep);


 QJsonArray qArray;

        // Iterate through the nlohmann::json array and build the QJsonArray
        for (const auto& obj : j3) {
            QJsonObject qObj;
            
            // Iterate through each key-value pair in the JSON object
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                // Convert nlohmann::json types to QVariant and then to QJsonValue
                QVariant variantValue = QVariant::fromValue(it.value());
                QJsonValue qJsonValue = QJsonValue::fromVariant(variantValue);

                // Insert the QJsonValue into the QJsonObject
                qObj[QString::fromStdString(it.key())] = qJsonValue;
            }

            // Append the QJsonObject to the QJsonArray
            qArray.append(qObj);
        };

        QLabel* tokenLabelSize = new QLabel("Total broadcasts " + QString::number(qArray.size()));
        newUiLayout->addWidget(tokenLabelSize);


		// Create a scroll area and set up a widget to contain the items
QScrollArea* scrollArea = new QScrollArea;
QWidget* scrollWidget = new QWidget;

// scrollWidget->resize(300,300);
scrollArea->setWidgetResizable(true);
scrollArea->setWidget(scrollWidget);

// Create a layout for the widget inside the scroll area
QVBoxLayout* scrollLayout = new QVBoxLayout(scrollWidget);

for (const auto& jsonObject : j3) {
    
        // QJsonObject jsonObject = jsonValue.toObject();
        if (jsonObject.contains("title")) {
            // Create a custom widget to represent each item
            QWidget* itemWidget = new QWidget;
            QVBoxLayout* itemLayout = new QVBoxLayout(itemWidget);

			// Create a group for title and scheduledTime
            QGroupBox* titleScheduledGroup = new QGroupBox;
            QVBoxLayout* titleScheduledLayout = new QVBoxLayout(titleScheduledGroup);

			// Set a fixed size for the QGroupBox
			titleScheduledGroup->setMinimumSize(220, 110); // Set the minimum width and height

            // Show title from jsonObject
             QString title = QString::fromStdString(jsonObject["title"].get<std::string>());
            QLabel* titleLabel = new QLabel(title);
			titleLabel->setWordWrap(true);
			titleLabel->setStyleSheet("QLabel{font-size: 17px;font-family: Arial;}"); 
            titleScheduledLayout->addWidget(titleLabel);


            // Show scheduledTime from jsonObject
            QString scheduledTimeStr = QString::fromStdString(jsonObject["scheduledTime"].get<std::string>());
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
			QObject::connect(SelectButton, &QPushButton::clicked, [this , jsonObject , tabWidget]() {
					currentBroadcast = jsonObject["id"].dump();
				

                	auto &global = GlobalMultiOutputConfig();
				
                	global.targets.clear();
					SaveMultiOutputConfig();
					LoadConfig();
					tab2Layout = 0;

			auto destinationsArray = jsonObject["destinations"];
			 auto firstDestination = destinationsArray.at(0);
			//  QJsonObject firstDestination = destinationsArray[0].toObject();
			 	obs_service_t *service = obs_frontend_get_streaming_service();
    			obs_data_t *settings = obs_service_get_settings(service);
    			// cout << obs_data_get_json_pretty(settings) << endl;
				QString url = firstDestination["url"].get<std::string>().c_str();
        		QString key = firstDestination["key"].get<std::string>().c_str();
				obs_data_set_string(settings, "key", key.toStdString().c_str());
				obs_data_set_string(settings, "server", url.toStdString().c_str());
    			obs_data_release(settings);

				
					for (size_t i = 1; i < destinationsArray.size(); ++i) {
            auto destination = destinationsArray.at(i);

            QString platformUserName = destination["platformUserName"].get<std::string>().c_str();
            QString platformTitle = destination["platformTitle"].get<std::string>().c_str();
            QString title = platformUserName + " / " + platformTitle;

            auto newid = GenerateId(global);
            auto target = std::make_shared<OutputTargetConfig>();
            target->id = newid;
            target->name = title.toStdString();
            target->serviceParam = {
                {"server", destination["url"].get<std::string>()},
                {"key", destination["key"].get<std::string>()}
            };
            target->syncStart = true;
            global.targets.emplace_back(target);

            auto pushwidget = createPushWidget(newid, container_);
            itemLayout_->addWidget(pushwidget);
            SaveConfig();
        }
				
				tabWidget->setCurrentIndex(1);
    	});

		
			titleScheduledLayout->addWidget(SelectButton);

			// Add the title and scheduledTime group to the item layout
            itemLayout->addWidget(titleScheduledGroup);



            // Add the custom widget to the scroll layout
            scrollLayout->addWidget(itemWidget);
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
        // Open the URL in the user's default web browser
        QDesktopServices::openUrl(QUrl("https://app.streamway.in/"));
				 });
// Create a container widget for the button layout
QWidget* buttonContainer = new QWidget;
buttonContainer->setLayout(buttonLayout);
newUiLayout->addWidget(buttonContainer);

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