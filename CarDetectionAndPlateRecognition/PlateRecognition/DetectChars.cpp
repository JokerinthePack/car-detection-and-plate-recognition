// DetectChars.cpp

#include "DetectChars.h"

// global değişkenler ///////////////////////////////////////////////////////////////////////////////
cv::Ptr<cv::ml::KNearest> kNearest = cv::ml::KNearest::create();

///////////////////////////////////////////////////////////////////////////////////////////////////
bool loadKNNDataAndTrainKNN(void) {

    // sınıflandırmaları ilişkilendirme ///////////////////////////////////////////////////

    cv::Mat matClassificationInts;              // sınıflandırma numaraları tek bir değişkende vektör olarak toplanır

    cv::FileStorage fsClassifications("classifications.xml", cv::FileStorage::READ);        // sınıflandırma dosyası açılır

    if (fsClassifications.isOpened() == false) {                                                        // dosya açılmazsa
        std::cout << "hata, siniflandirma ögrenimi dosyasi acilamiyor, program kapatiliyor\n\n";        // mesaj gösterilir
		return(false);                                                                                  // programdan çıkılır
    }

    fsClassifications["classifications"] >> matClassificationInts;          // sınıflandırma kısmı Mat sınıflandırma değişkenine atanır
    fsClassifications.release();                                            // sınıflandırma dosyası kapatılır

                                                                            // öğrenim görsellerini atama ////////////////////////////////////////////////////////////

    cv::Mat matTrainingImagesAsFlattenedFloats;         // birden fazla görsel tek bir görsele vektör olarak atanır

    cv::FileStorage fsTrainingImages("images.xml", cv::FileStorage::READ);              // öğrenim görselleri dosyası açılır

    if (fsTrainingImages.isOpened() == false) {                                                 // dosya açılmazsa
        std::cout << "hata, ogrenim gorselleri dosyasi acilamiyor, program kapatiliyor\n\n";         // mesaj gösterilir
	    return(false);                                                                          // programdan çıkılır
    }

    fsTrainingImages["images"] >> matTrainingImagesAsFlattenedFloats;           // öğrenim görselleri kısmı Mat görsel değişkenine atanır
    fsTrainingImages.release();                                                 // öğrenim görselleri dosyası kapatılır

                                                                                // öğrenme //////////////////////////////////////////////////////////////////////////////

                                                                                // öğrenmeye başlanır 
                                                                                // tüm parametreler tek bir Mat değişkenine atanır
    kNearest->setDefaultK(1);

    kNearest->train(matTrainingImagesAsFlattenedFloats, cv::ml::ROW_SAMPLE, matClassificationInts);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<PossiblePlate> detectCharsInPlates(std::vector<PossiblePlate> &vectorOfPossiblePlates) {
    int intPlateCounter = 0;				// adımlar gösterilir
    cv::Mat imgContours;
    std::vector<std::vector<cv::Point> > contours;
    cv::RNG rng;

    if (vectorOfPossiblePlates.empty()) {               // olası plakalar vektörü boşsa
        return(vectorOfPossiblePlates);                 // geri dönülür
    }
    // vektörde en az bir plaka bulunuyorsa

    for (auto &possiblePlate : vectorOfPossiblePlates) {            // her bir olası plaka için

        preprocess(possiblePlate.imgPlate, possiblePlate.imgGrayscale, possiblePlate.imgThresh);        // griölçek ve eşik görselleri çıkarılır

#ifdef SHOW_STEPS
        cv::imshow("5a", possiblePlate.imgPlate);
        cv::imshow("5b", possiblePlate.imgGrayscale);
        cv::imshow("5c", possiblePlate.imgThresh);
#endif	// adım gösterilir

        // daha iyi görüntü ve tanımlama için görüntü %60 büyütülür
        cv::resize(possiblePlate.imgThresh, possiblePlate.imgThresh, cv::Size(), 1.6, 1.6);

        // gri alanlardan kurtulmak için tekrar eşikleme yapılır
        cv::threshold(possiblePlate.imgThresh, possiblePlate.imgThresh, 0.0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);

#ifdef SHOW_STEPS
        cv::imshow("5d", possiblePlate.imgThresh);
#endif	// adım gösterilir

        // plakadaki tüm olası karakterler bulunur
        // tüm kontürler bulunur, ardından sadece karakter olabilecek kontürler tutulur (henüz karakterler arası karşılaştırma yapılmaz.)
        std::vector<PossibleChar> vectorOfPossibleCharsInPlate = findPossibleCharsInPlate(possiblePlate.imgGrayscale, possiblePlate.imgThresh);

#ifdef SHOW_STEPS
        imgContours = cv::Mat(possiblePlate.imgThresh.size(), CV_8UC3, SCALAR_BLACK);
        contours.clear();

        for (auto &possibleChar : vectorOfPossibleCharsInPlate) {
            contours.push_back(possibleChar.contour);
        }

        cv::drawContours(imgContours, contours, -1, SCALAR_WHITE);

        cv::imshow("6", imgContours);
#endif	// adım gösterilir

        // tüm olası karakterleri içeren vektörün içinden, plaka içinde eşleşen karakter grupları tespit edilir
        std::vector<std::vector<PossibleChar> > vectorOfVectorsOfMatchingCharsInPlate = findVectorOfVectorsOfMatchingChars(vectorOfPossibleCharsInPlate);

#ifdef SHOW_STEPS
        imgContours = cv::Mat(possiblePlate.imgThresh.size(), CV_8UC3, SCALAR_BLACK);

        contours.clear();

        for (auto &vectorOfMatchingChars : vectorOfVectorsOfMatchingCharsInPlate) {
            int intRandomBlue = rng.uniform(0, 256);
            int intRandomGreen = rng.uniform(0, 256);
            int intRandomRed = rng.uniform(0, 256);

            for (auto &matchingChar : vectorOfMatchingChars) {
                contours.push_back(matchingChar.contour);
            }
            cv::drawContours(imgContours, contours, -1, cv::Scalar((double)intRandomBlue, (double)intRandomGreen, (double)intRandomRed));
        }
        cv::imshow("7", imgContours);
#endif	// adım gösterilir

        if (vectorOfVectorsOfMatchingCharsInPlate.size() == 0) {                // eşleşen karakter grubu bulunamazsa
#ifdef SHOW_STEPS
            std::cout << intPlateCounter << " nolu plakada bulunan karakterler " << " = (sifir), bir gorsele tiklayin ve bir tusa basin . . ." << std::endl;
            intPlateCounter++;
            cv::destroyWindow("8");
            cv::destroyWindow("9");
            cv::destroyWindow("10");
            cv::waitKey(0);
#endif	// adım gösterilir
            possiblePlate.strChars = "";            // plaka dizisinin değişken sayısı boş diziye atanır
            continue;                               // for döngüsünün başına geri dönülür
        }

        for (auto &vectorOfMatchingChars : vectorOfVectorsOfMatchingCharsInPlate) {                                         // mevcut plakadaki her bir eşleşen karakter vektörü için
            std::sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), PossibleChar::sortCharsLeftToRight);      // karakterler soldan sağa sıralanır
            vectorOfMatchingChars = removeInnerOverlappingChars(vectorOfMatchingChars);                                     // ve üstüste binen karakterler çıkartılır
        }

#ifdef SHOW_STEPS
        imgContours = cv::Mat(possiblePlate.imgThresh.size(), CV_8UC3, SCALAR_BLACK);

        for (auto &vectorOfMatchingChars : vectorOfVectorsOfMatchingCharsInPlate) {
            int intRandomBlue = rng.uniform(0, 256);
            int intRandomGreen = rng.uniform(0, 256);
            int intRandomRed = rng.uniform(0, 256);

            contours.clear();

            for (auto &matchingChar : vectorOfMatchingChars) {
                contours.push_back(matchingChar.contour);
            }
            cv::drawContours(imgContours, contours, -1, cv::Scalar((double)intRandomBlue, (double)intRandomGreen, (double)intRandomRed));
        }
        cv::imshow("8", imgContours);
#endif	// adım gösterilir

        // tüm olası plakalar içinden, en çok olası eşleşen karaktere sahip olan vektör esas vektör olarak belirlenir
        unsigned int intLenOfLongestVectorOfChars = 0;
        unsigned int intIndexOfLongestVectorOfChars = 0;
        // en fazla karaktere sahip vektörün indeksi alınır
        for (unsigned int i = 0; i < vectorOfVectorsOfMatchingCharsInPlate.size(); i++) {
            if (vectorOfVectorsOfMatchingCharsInPlate[i].size() > intLenOfLongestVectorOfChars) {
                intLenOfLongestVectorOfChars = vectorOfVectorsOfMatchingCharsInPlate[i].size();
                intIndexOfLongestVectorOfChars = i;
            }
        }
        // en çok olası eşleşen karaktere sahip olan vektör esas vektör olarak belirlenir
        std::vector<PossibleChar> longestVectorOfMatchingCharsInPlate = vectorOfVectorsOfMatchingCharsInPlate[intIndexOfLongestVectorOfChars];

#ifdef SHOW_STEPS
        imgContours = cv::Mat(possiblePlate.imgThresh.size(), CV_8UC3, SCALAR_BLACK);

        contours.clear();

        for (auto &matchingChar : longestVectorOfMatchingCharsInPlate) {
            contours.push_back(matchingChar.contour);
        }
        cv::drawContours(imgContours, contours, -1, SCALAR_WHITE);

        cv::imshow("9", imgContours);
#endif	// adım gösterilir

        // plakadaki en uzun vektöre karakter tanımlaması uygulanır
        possiblePlate.strChars = recognizeCharsInPlate(possiblePlate.imgThresh, longestVectorOfMatchingCharsInPlate);

#ifdef SHOW_STEPS
        std::cout << intPlateCounter << " nolu plakada bulunan karakterler " << " = " << possiblePlate.strChars << ", bir gorsele tiklayin ve bir tusa basin . . ." << std::endl;
        intPlateCounter++;
        cv::waitKey(0);
#endif	// adım gösterilir

    }   // olası plakaları tarama işlemi bitirilir

#ifdef SHOW_STEPS
    std::cout << std::endl << "karakter tespiti tamamlandi, bir gorsele tiklayin ve bir tusa basin . . ." << std::endl;
    cv::waitKey(0);
#endif	// adım gösterilir

    return(vectorOfPossiblePlates);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<PossibleChar> findPossibleCharsInPlate(cv::Mat &imgGrayscale, cv::Mat &imgThresh) {
    std::vector<PossibleChar> vectorOfPossibleChars;                            // geri dönme değeri

    cv::Mat imgThreshCopy;

    std::vector<std::vector<cv::Point> > contours;

    imgThreshCopy = imgThresh.clone();				// eşikli görüntünün kopyası alınır (findContours fonksiyonu görselde değişiklik yapmaktadır.)

    cv::findContours(imgThreshCopy, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);        // plakadaki tüm kontürler bulunur

    for (auto &contour : contours) {                            // her bir kontür için
        PossibleChar possibleChar(contour);

        if (checkIfPossibleChar(possibleChar)) {                // kontür olası bir karakter ise
            vectorOfPossibleChars.push_back(possibleChar);      // olası karakterler vektörüne ekleme yapılır (henüz diğer karakterler ile karşılaştırma yapılmamaktadır)
        }
    }

    return(vectorOfPossibleChars);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool checkIfPossibleChar(PossibleChar &possibleChar) {
    // olası karakter tespiti için basit bir kontür taraması yapılır
    if (possibleChar.boundingRect.area() > MIN_PIXEL_AREA &&
        possibleChar.boundingRect.width > MIN_PIXEL_WIDTH && possibleChar.boundingRect.height > MIN_PIXEL_HEIGHT &&
        MIN_ASPECT_RATIO < possibleChar.dblAspectRatio && possibleChar.dblAspectRatio < MAX_ASPECT_RATIO) {
        return(true);
    }
    else {
        return(false);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<std::vector<PossibleChar> > findVectorOfVectorsOfMatchingChars(const std::vector<PossibleChar> &vectorOfPossibleChars) {
    // bir vektör içerisindeki tüm olası karakterlere bakılır
    // buradaki amaç, karakterlerden oluşan bir vektörü, eşleşen karakter vektörlerin den oluşan, daha büyük bir vektöre dönüştürmektir
    // böylece herhangi bir eşleşme grubu içinde bulunmayan karakterlerin incelenmesine gerek kalmamaktadır
    std::vector<std::vector<PossibleChar> > vectorOfVectorsOfMatchingChars;             // geri dönme değeri

    for (auto &possibleChar : vectorOfPossibleChars) {                  // karakter vektörü içindeki her bir karakter için

                                                                        // mevcut karakter ile eşleşen tüm karakterler tespit edilir
        std::vector<PossibleChar> vectorOfMatchingChars = findVectorOfMatchingChars(possibleChar, vectorOfPossibleChars);

        vectorOfMatchingChars.push_back(possibleChar);          // mevcut karakter, mevcut olası karakterler vektörüne eklenir

                                                                // mevcut vektör bir plaka oluşturacak kadar uzun değil ise
        if (vectorOfMatchingChars.size() < MIN_NUMBER_OF_MATCHING_CHARS) {
            continue;                       // for döngüsünün başına dönülür ve bir sonraki karakter için tekrar deneme yapılır
                                            
        }
        // bu noktaya kadar gelinmesi, mevcut vektörün "grup" olarak nitelendirilebildiği anlamına gelmektedir 
        vectorOfVectorsOfMatchingChars.push_back(vectorOfMatchingChars);            // eşleşen karakter vektörlerinden oluşan büyük vektöre ekleme yap

                                                                                    // aynı karakterlerin tekrar kullanılmaması için mevcut vektör büyük vektörden çıkarılır ve
                                                                                    // orjinal büyük vektörün değişmemesi için yeni bir adet büyük vektör oluşturulur
        std::vector<PossibleChar> vectorOfPossibleCharsWithCurrentMatchesRemoved;

        for (auto &possChar : vectorOfPossibleChars) {
            if (std::find(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), possChar) == vectorOfMatchingChars.end()) {
                vectorOfPossibleCharsWithCurrentMatchesRemoved.push_back(possChar);
            }
        }
        // tekrar çağırmadan sonuç alınabilmesi için yeni bir vektörler vektörü oluşturulur
        std::vector<std::vector<PossibleChar> > recursiveVectorOfVectorsOfMatchingChars;

        // tekrar çağırma
        recursiveVectorOfVectorsOfMatchingChars = findVectorOfVectorsOfMatchingChars(vectorOfPossibleCharsWithCurrentMatchesRemoved);	// tekrar çağırma !!

        for (auto &recursiveVectorOfMatchingChars : recursiveVectorOfVectorsOfMatchingChars) {      // tekrar çağırma tarafından bulunan her karakter vektörü için
            vectorOfVectorsOfMatchingChars.push_back(recursiveVectorOfMatchingChars);               // orjinal büyük vektöre ekleme yapılır
        }

        break;		// for döngüsünden çıkılır
    }

    return(vectorOfVectorsOfMatchingChars);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<PossibleChar> findVectorOfMatchingChars(const PossibleChar &possibleChar, const std::vector<PossibleChar> &vectorOfChars) {
    // buradaki amaç elde bulunan her bir olası karakter ve olası karakterler vektörü için
    // bir eşleşme olması durumunda, eşleşen karakterlerin vektörlere atanmasını sağlamaktır
    std::vector<PossibleChar> vectorOfMatchingChars;                // geri dönüş değeri

    for (auto &possibleMatchingChar : vectorOfChars) {              // büyük vektördeki her bir karakter için

                                                                    // kontrolü yapılan karakter ile vektör içinde eşleşme için bakılan karakter birebir aynı ise
        if (possibleMatchingChar == possibleChar) {
            // mevcut karakterin iki kez eklenmemesi için vektöre eklenme yapılmaz
            continue;           // ve for döngüsünün başına geri dönülür
        }
        // karakter eşleşme tespiti için işlemler yapılır
        double dblDistanceBetweenChars = distanceBetweenChars(possibleChar, possibleMatchingChar);
        double dblAngleBetweenChars = angleBetweenChars(possibleChar, possibleMatchingChar);
        double dblChangeInArea = (double)abs(possibleMatchingChar.boundingRect.area() - possibleChar.boundingRect.area()) / (double)possibleChar.boundingRect.area();
        double dblChangeInWidth = (double)abs(possibleMatchingChar.boundingRect.width - possibleChar.boundingRect.width) / (double)possibleChar.boundingRect.width;
        double dblChangeInHeight = (double)abs(possibleMatchingChar.boundingRect.height - possibleChar.boundingRect.height) / (double)possibleChar.boundingRect.height;

        // eşleşme mevcudiyeti kontrol edilir
        if (dblDistanceBetweenChars < (possibleChar.dblDiagonalSize * MAX_DIAG_SIZE_MULTIPLE_AWAY) &&
            dblAngleBetweenChars < MAX_ANGLE_BETWEEN_CHARS &&
            dblChangeInArea < MAX_CHANGE_IN_AREA &&
            dblChangeInWidth < MAX_CHANGE_IN_WIDTH &&
            dblChangeInHeight < MAX_CHANGE_IN_HEIGHT) {
            vectorOfMatchingChars.push_back(possibleMatchingChar);      // eşleşme mevcutsa, vektöre ekleme yapılır
        }
    }

    return(vectorOfMatchingChars);          // sonuca geri dönülür
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// İki karakter arası mesafe hesabı için Pisagor Teoremi uygulanır
double distanceBetweenChars(const PossibleChar &firstChar, const PossibleChar &secondChar) {
    int intX = abs(firstChar.intCenterX - secondChar.intCenterX);
    int intY = abs(firstChar.intCenterY - secondChar.intCenterY);

    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// karakterler arası açı hesabı için geometrik hesaplamalar yapılır
double angleBetweenChars(const PossibleChar &firstChar, const PossibleChar &secondChar) {
    double dblAdj = abs(firstChar.intCenterX - secondChar.intCenterX);
    double dblOpp = abs(firstChar.intCenterY - secondChar.intCenterY);

    double dblAngleInRad = atan(dblOpp / dblAdj);

    double dblAngleInDeg = dblAngleInRad * (180.0 / CV_PI);

    return(dblAngleInDeg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// üstüste binen ya da birbirine fazla yakın iki karakter var ise küçük olan silinir
// buradaki amaç aynı karakter için aynı kontürler bulunması durumunda iki kez aynı karakteri yazmayı engellemektir,
// örneğin "O" harfi için hem iç hem de dış çemberler kontür olarak alınabilir fakat karakterin bir kere yazılması gerekir
std::vector<PossibleChar> removeInnerOverlappingChars(std::vector<PossibleChar> &vectorOfMatchingChars) {
    std::vector<PossibleChar> vectorOfMatchingCharsWithInnerCharRemoved(vectorOfMatchingChars);

    for (auto &currentChar : vectorOfMatchingChars) {
        for (auto &otherChar : vectorOfMatchingChars) {
            if (currentChar != otherChar) {                         // mevcut ve öteki karakter aynı karakterler değil ise . . .
                                                                    // mevcut ve öteki karakterin merkezleri yaklaşık olarak aynı noktadaysa . . .
                if (distanceBetweenChars(currentChar, otherChar) < (currentChar.dblDiagonalSize * MIN_DIAG_SIZE_MULTIPLE_AWAY)) {
                    // bu noktaya gelindi ise üstüste binen karakterler var demektir
                    // küçük olan karakter tespit edilir, bu karakter daha önce silinmediyse, bu adımda silinir

                    // mevcut karakter diğer karakterden küçük ise
                    if (currentChar.boundingRect.area() < otherChar.boundingRect.area()) {
                        // yineleyici kullanarak vektör içinde kararter aranır
                        std::vector<PossibleChar>::iterator currentCharIterator = std::find(vectorOfMatchingCharsWithInnerCharRemoved.begin(), vectorOfMatchingCharsWithInnerCharRemoved.end(), currentChar);
                        // yineleyici vektörün sonuna ulaşmaz ise, karakter vektör içinde mevcut demektir
                        if (currentCharIterator != vectorOfMatchingCharsWithInnerCharRemoved.end()) {
                            vectorOfMatchingCharsWithInnerCharRemoved.erase(currentCharIterator);       // karakteri sil
                        }
                    }
                    else {        // diğer karakter mevcuk karakterden küçük ise
                                  // yineleyici kullanarak vektör içinde kararter aranır
                        std::vector<PossibleChar>::iterator otherCharIterator = std::find(vectorOfMatchingCharsWithInnerCharRemoved.begin(), vectorOfMatchingCharsWithInnerCharRemoved.end(), otherChar);
                        // yineleyici vektörün sonuna ulaşmaz ise, karakter vektör içinde mevcut demektir
                        if (otherCharIterator != vectorOfMatchingCharsWithInnerCharRemoved.end()) {
                            vectorOfMatchingCharsWithInnerCharRemoved.erase(otherCharIterator);         // karakteri sil
                        }
                    }
                }
            }
        }
    }

    return(vectorOfMatchingCharsWithInnerCharRemoved);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// esas karakter tanımlama işlemi burada uygulanır
std::string recognizeCharsInPlate(cv::Mat &imgThresh, std::vector<PossibleChar> &vectorOfMatchingChars) {
    std::string strChars;               // geri dönüş değeri

    cv::Mat imgThreshColor;

    // karakterler soldan sağa sıralanır
    std::sort(vectorOfMatchingChars.begin(), vectorOfMatchingChars.end(), PossibleChar::sortCharsLeftToRight);

    cv::cvtColor(imgThresh, imgThreshColor, CV_GRAY2BGR);       // kontürlerin çizilebilmesi için eşikli görüntünün renkli versiyonu oluşturulur

    for (auto &currentChar : vectorOfMatchingChars) {           // plakada bulunan her karakter için
        cv::rectangle(imgThreshColor, currentChar.boundingRect, SCALAR_GREEN, 2);       // karakter etrafına yeşil kutu çizilir

        cv::Mat imgROItoBeCloned = imgThresh(currentChar.boundingRect);                 // kutu içinde kalan bölgenin görüntüsü alınır

        cv::Mat imgROI = imgROItoBeCloned.clone();      // yeniden boyutlandırma sonrası orjinal görüntünün değişmemesi için kopyası alınır

        cv::Mat imgROIResized;
        // karakter tanımlaması için yeniden boyutlandırma yapılır
        cv::resize(imgROI, imgROIResized, cv::Size(RESIZED_CHAR_IMAGE_WIDTH, RESIZED_CHAR_IMAGE_HEIGHT));

        cv::Mat matROIFloat;

        imgROIResized.convertTo(matROIFloat, CV_32FC1);         // findNearest in çağırılabilmesi için Mat float a çevirilir

        cv::Mat matROIFlattenedFloat = matROIFloat.reshape(1, 1);       // matris tek sıraya indirilir

        cv::Mat matCurrentChar(0, 0, CV_32F);                   // mevcut karakter Mat a atanır (findNearest için)

        kNearest->findNearest(matROIFlattenedFloat, 1, matCurrentChar);     // findNearest çağırılır

        float fltCurrentChar = (float)matCurrentChar.at<float>(0, 0);       // mevcut karakter Mat tan float a çevirilir

        strChars = strChars + char(int(fltCurrentChar));        // mevcut karakter diziye eklenir
    }

#ifdef SHOW_STEPS
    cv::imshow("10", imgThreshColor);
#endif	// adım gösterilir

    return(strChars);               // sonuca geri dönülür
}

