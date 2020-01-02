// Main.cpp

#include "Main.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {
	
	
			bool blnKNNTrainingSuccessful = loadKNNDataAndTrainKNN();           // KNN öğrenimi başlatılır

			if (blnKNNTrainingSuccessful == false) {                            // KNN öğrenimi çalışmazsa
																				// hata mesajı gösterilir
				std::cout << std::endl << std::endl << "hata: hata: KNN ogrenimi başarili olamadi" << std::endl << std::endl;
				return(0);                                                      // ve program kapatılır
			}

			cv::Mat imgOriginalScene;           // input görseli

			imgOriginalScene = cv::imread("CAR.png");         // görsel açılır

			if (imgOriginalScene.empty()) {                             // görsel açılmazsa
				std::cout << "hata: gorsel dosyadan okunamadi\n\n";     // komut satırında hata mesajı gösterilir
				_getch();                                               // Windows kullanılmıyorsa bu satır değiştirilir
				return(0);                                              // ve program kapatılır
			}

			std::vector<PossiblePlate> vectorOfPossiblePlates = detectPlatesInScene(imgOriginalScene);          // plakalar tespit edilir

			vectorOfPossiblePlates = detectCharsInPlates(vectorOfPossiblePlates);                               // plakalardaki karakterler tespit edilir

			cv::imshow("imgOriginalScene", imgOriginalScene);           // araç görseli gösterilir

			if (vectorOfPossiblePlates.empty()) {                                               // plaka bulunamazsa
				std::cout << std::endl << "plaka tespit edilemedi" << std::endl;       // kullanıcı uyarılır
			}
			else {                                                                            // bulunursa
																							  // olası plakalar vektörler halinde belirlenir

																							  // vektörler karakter sayılarına göre azalan şeklinde sıralanır
				std::sort(vectorOfPossiblePlates.begin(), vectorOfPossiblePlates.end(), PossiblePlate::sortDescendingByNumberOfChars);

				// en çok karaktere sahip olan vektör esas plaka olarak tanımlanır
				PossiblePlate licPlate = vectorOfPossiblePlates.front();

				cv::imshow("imgPlate", licPlate.imgPlate);            // plakanın orjinali ve eşikli görseli gösterilir
				cv::imshow("imgThresh", licPlate.imgThresh);

				if (licPlate.strChars.length() == 0) {                                                      // plakada karakter tespit edilemezse
					std::cout << std::endl << "karakter tespiti yapilamadi" << std::endl << std::endl;      // mesaj gösterilir
					return(0);                                                                              // ve program kapatılır
				}

				drawRedRectangleAroundPlate(imgOriginalScene, licPlate);                // plaka etrafına kırmızı dörtgen çizilir

				std::cout << std::endl << "Arac Plakasi = " << licPlate.strChars << std::endl;     // plaka çıktı olarak yazdırılır
				std::cout << std::endl << "-----------------------------------------" << std::endl;

				writeLicensePlateCharsOnImage(imgOriginalScene, licPlate);              // plaka görsele yazdırılır

				cv::imshow("imgOriginalScene", imgOriginalScene);                       // araç görseli tekrar gösterilir

				cv::imwrite("imgOriginalScene.png", imgOriginalScene);                  // araç görseli kaydedilir
			}
		
			
			cv::waitKey(0);

			while (1);
	
			return(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawRedRectangleAroundPlate(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
    cv::Point2f p2fRectPoints[4];

    licPlate.rrLocationOfPlateInScene.points(p2fRectPoints);            // dörtgen köşeleri belirlenir

    for (int i = 0; i < 4; i++) {                                       // kırmızı çizgiler çekilir
        cv::line(imgOriginalScene, p2fRectPoints[i], p2fRectPoints[(i + 1) % 4], SCALAR_RED, 2);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void writeLicensePlateCharsOnImage(cv::Mat &imgOriginalScene, PossiblePlate &licPlate) {
    cv::Point ptCenterOfTextArea;                   // plakanın yazılacağı alanın merkezi
    cv::Point ptLowerLeftTextOrigin;                // sol alt köşesi

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;                              // yazı fontu belirlenir
    double dblFontScale = (double)licPlate.imgPlate.rows / 30.0;            // plaka yüksekliğine göre font ölçeklenir
    int intFontThickness = (int)std::round(dblFontScale * 1.5);             // ölçeğe göre kalınlık belirlenir
    int intBaseline = 0;

    cv::Size textSize = cv::getTextSize(licPlate.strChars, intFontFace, dblFontScale, intFontThickness, &intBaseline);      // getTextSize çağırılır

    ptCenterOfTextArea.x = (int)licPlate.rrLocationOfPlateInScene.center.x;         // yazı alanının yatay konumu plakaya göre ayarlanır

    if (licPlate.rrLocationOfPlateInScene.center.y < (imgOriginalScene.rows * 0.75)) {      // plaka görselin yatay üst üç çeyreğinden birindeyse
                                                                                            // plaka alt kısma yazılır
        ptCenterOfTextArea.y = (int)std::round(licPlate.rrLocationOfPlateInScene.center.y) + (int)std::round((double)licPlate.imgPlate.rows * 1.6);
    }
    else {                                                                                // plaka en altta bulunan çeyrekteyse
                                                                                          // üst kısma yazılır
        ptCenterOfTextArea.y = (int)std::round(licPlate.rrLocationOfPlateInScene.center.y) - (int)std::round((double)licPlate.imgPlate.rows * 1.6);
    }

    ptLowerLeftTextOrigin.x = (int)(ptCenterOfTextArea.x - (textSize.width / 2));           // yazı alanının sol alt orjini
    ptLowerLeftTextOrigin.y = (int)(ptCenterOfTextArea.y + (textSize.height / 2));          // merkez, uzunluk ve yüksekliğe göre belirlenir

                                                                                            // yazı görsele yazdırılır
    cv::putText(imgOriginalScene, licPlate.strChars, ptLowerLeftTextOrigin, intFontFace, dblFontScale, SCALAR_YELLOW, intFontThickness);
}


