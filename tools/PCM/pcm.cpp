#include <iostream>
#include <string>
#include <cstdio>
#include <vector>
#include <cmath>

using namespace std;

typedef struct RiffFmt {
   int size;

   short type;
   short numChannels;

   int sampleRate;
   int byteRate;

   short blockAlign;
   short bitDepth;
} RiffFmt;

typedef struct RiffHeader {
    RiffFmt fmt;
} RiffHeader;



void parseRiff(FILE *fp, RiffHeader *header) {
    fseek(fp, 4, SEEK_SET);

    int fsize;
    fread(&fsize, sizeof(int), 1, fp);

    cout << "File Size: " << fsize + 8 << " bytes = " << (fsize + 8) / 1024  << "KB" << endl;

    fseek(fp, 4, SEEK_CUR);

    while (1) {
        char temp[4] = {0};
        fread(temp, 1, 4, fp);
        if (temp[0] == 'f' && temp[1] == 'm' && temp[2] == 't') {
            fread(&header->fmt, sizeof(RiffFmt), 1, fp);
            fseek(fp, header->fmt.size - sizeof(RiffFmt) + 4, SEEK_CUR);
        } else if (temp[0] == 'd' && temp[1] == 'a' && temp[2] == 't' && temp[3] == 'a') {
            int size;
            fread(&size, 4, 1, fp);

            return;
        }else {
            int size;
            fread(&size, 4, 1, fp);

            fseek(fp, size, SEEK_CUR);
        }
    }
}

void parseWaveContent(FILE *fp, const RiffHeader &header, vector<pair<int, int>> &samples) {
    while (!feof(fp)) {
        char L[3], R[3];

        fread(L, header.fmt.bitDepth / 8, 1, fp);
        fread(R, header.fmt.bitDepth / 8, 1, fp);

        samples.push_back({L[0] + L[1] << 8 + L[2] << 16, R[0] + R[1] << 8 + R[2] << 16});
    }
}

int main() {
    string fn;
    cout << "Input File Name: ";
    cin >> fn;

    FILE *fp = fopen(fn.c_str(), "rb");

    RiffHeader header;
    parseRiff(fp, &header);

    vector<pair<int, int>> samples;
    parseWaveContent(fp, header, samples);

    for (auto &[l, r]: samples) {
        cout << l << ", " << r << endl;
        getchar();
    }


    fclose(fp);
    return 0;
}
