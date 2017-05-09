#include "../config.h"
#include "../writing.h"
#include <gtest/gtest.h>
using namespace std;
TEST(Smoke_Test, Empty_Conditions){
    char** options = new char*[7];
    options[0] = const_cast<char*>("BAG");
    options[1] = const_cast<char*>("-f");
    options[2] = const_cast<char*>("../example.bag");
    options[3] = const_cast<char*>("-c");
    options[4] = const_cast<char*>("good");
    options[5] = const_cast<char*>("-o");
    options[6] = const_cast<char*>("bad");
    Config conf(7, options);
    delete[] options;
    Bag_header bh;
    vector<Chunk> chunks;
    ifstream in(conf.in_filename, ios::binary);
    ofstream out(conf.out_filename_correct, ios::binary);

    read_BAG(conf,chunks,bh,in,out);
    Bag_header first = bh;

    Bag_header bh_other = bh;
    vector<Chunk> chunks_other = chunks;

    conf.correct_conditions = true;
    conf.parseBag(chunks, bh, in);
    writingBag(chunks, bh, conf, in, out);
    ASSERT_TRUE(bh.index_pos == first.index_pos);
    ASSERT_TRUE(bh.chunk_count == first.chunk_count);
    ASSERT_TRUE(bh.conn_count == first.conn_count);
    conf.correct_conditions = false;
    conf.parseBag(chunks_other, bh_other, in);
    out = ofstream(conf.out_filename_other, ios::binary);
    writingBag(chunks_other, bh_other, conf, in, out);
    system("rm good");
    system("rm bad");
}

TEST(Smoke_Test, Full_Conditions){
    char** options = new char*[11];
    options[0] = const_cast<char*>("BAG");
    options[1] = const_cast<char*>("-f");
    options[2] = const_cast<char*>("../example.bag");
    options[3] = const_cast<char*>("-c");
    options[4] = const_cast<char*>("good");
    options[5] = const_cast<char*>("-o");
    options[6] = const_cast<char*>("bad");
    options[7] = const_cast<char*>("--time");
    options[8] = const_cast<char*>("[0:0,4000000000:0]");
    options[9] = const_cast<char*>("--topic");
    options[10] = const_cast<char*>("/rosout");
    Config conf(11, options);
    delete[] options;
    Bag_header bh;
    vector<Chunk> chunks;
    ifstream in(conf.in_filename, ios::binary);
    ofstream out(conf.out_filename_correct, ios::binary);

    read_BAG(conf,chunks,bh,in,out);

    Bag_header bh_other = bh;
    vector<Chunk> chunks_other = chunks;

    conf.correct_conditions = true;
    conf.parseBag(chunks, bh, in);

    writingBag(chunks, bh, conf, in, out);
    ASSERT_TRUE(bh.conn_count);
    ASSERT_TRUE(bh.chunk_count);
    ASSERT_TRUE(bh.index_pos > 4117);
    conf.correct_conditions = false;
    conf.parseBag(chunks_other, bh_other, in);

    out = ofstream(conf.out_filename_other, ios::binary);
    writingBag(chunks_other, bh_other, conf, in, out);
    ASSERT_TRUE(bh_other.conn_count);
    ASSERT_TRUE(bh_other.chunk_count);
    ASSERT_TRUE(bh_other.index_pos > 4117);
    system("rm good");
    system("rm bad");
}


TEST(Smoke_Test, One_Message){
    char** options = new char*[11];
    options[0] = const_cast<char*>("BAG");
    options[1] = const_cast<char*>("-f");
    options[2] = const_cast<char*>("../example.bag");
    options[3] = const_cast<char*>("-c");
    options[4] = const_cast<char*>("good");
    options[5] = const_cast<char*>("-o");
    options[6] = const_cast<char*>("bad");
    options[7] = const_cast<char*>("--time");
    options[8] = const_cast<char*>("[1396293887:844783943,1396293887:844783943]");
    options[9] = const_cast<char*>("--topic");
    options[10] = const_cast<char*>(".*");
    Config conf(11, options);
    delete[] options;
    Bag_header bh;
    vector<Chunk> chunks;
    ifstream in(conf.in_filename, ios::binary);
    ofstream out(conf.out_filename_correct, ios::binary);

    read_BAG(conf,chunks,bh,in,out);
    Bag_header first = bh;
    int mess_count = 0;
    int mess_count1 = 0;
    int mess_count2 = 0;
    Bag_header bh_other = bh;
    vector<Chunk> chunks_other = chunks;

    conf.correct_conditions = true;
    for(int i = 0; i < chunks[0].info.data.size(); i++){
        mess_count1 += chunks[0].info.data[i].second;
    }
    conf.parseBag(chunks, bh, in);
    for(int i = 0; i < chunks[0].info.data.size(); i++){
        mess_count += chunks[0].info.data[i].second;
    }
    writingBag(chunks, bh, conf, in, out);
    ASSERT_TRUE(mess_count == 1);
    conf.correct_conditions = false;
    conf.parseBag(chunks_other, bh_other, in);
    for(int i = 0; i < chunks_other[0].info.data.size(); i++){
        mess_count2 += chunks_other[0].info.data[i].second;
    }
    ASSERT_TRUE(mess_count1 == mess_count2 + 1);
    out = ofstream(conf.out_filename_other, ios::binary);
    writingBag(chunks_other, bh_other, conf, in, out);
    system("rm good");
    system("rm bad");
}
