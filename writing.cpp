//
// Created by iisus on 30.04.2017.
//
#include "writing.h"

using namespace std;

void write_data(Header& h, std::ifstream& in, std::ofstream& out){
    in.seekg(h.data_begin, ios::beg);
    for(int32_t i = 0; i < h.data_len; i++){
        int8_t c;
        in.read((char*)&c, sizeof(int8_t));
        out.write((char*)&c, sizeof(int8_t));
    }
}

void write_index_data(Index_data& idh, ifstream& in, ofstream& out){
    int32_t n = idh.data.size();
    n *= 12;
    n/=12;
    for(int32_t i = 0; i < n; i++){
        out.write((char*)&idh.data[i].first.first, sizeof(uint32_t));
        out.write((char*)&idh.data[i].first.second, sizeof(uint32_t));
        out.write((char*)&idh.data[i].second, sizeof(int32_t));
    }
}

void write_chunk_info_data(Chunk_info& cih, ifstream& in, ofstream& out){
    int32_t n = cih.data.size();
    n *= 8;
    n/=8;
    for(int32_t i = 0; i < cih.data.size(); i++){
        out.write((char*)&cih.data[i].first, sizeof(int32_t));
        out.write((char*)&cih.data[i].second, sizeof(int32_t));
    }
}

void writing_chunk(Chunk& ch, std::ifstream& in, std::ofstream& out, std::map<int32_t, Connection>& unique_connections){
    if(ch.compression == "bz2")
        ch.compression = "bz2";//TODO
    ifstream unarch(ch.unarch_file, ios::binary);
    out << ch;
    int32_t pos = 0;
    for(int i = 0; i < ch.connections_info.size(); i++){
        if(ch.connections_to_write[ch.connections_info[i].conn]){
            out << unique_connections[ch.connections_info[i].conn];
            write_data(unique_connections[ch.connections_info[i].conn], in, out);
            pos += unique_connections[ch.connections_info[i].conn].all_size();
        }
        for(int j = 0; j < ch.connections_info[i].data.size(); j++){
            Message_header mh;
            if(ch.compression == "bz2"){
                unarch.seekg(ch.connections_info[i].data[j].second, ios::beg);
                unarch >> mh;
            }
            else{
                in.seekg(ch.data_begin + ch.connections_info[i].data[j].second, ios::beg);
                in >> mh;
            }
            out << mh;
            ch.connections_info[i].data[j].second = pos;
            pos += mh.all_size();
            if(ch.compression == "bz2")
                write_data(mh, unarch, out);
            else
                write_data(mh, in, out);
        }
    }
    for(int i = 0; i < ch.connections_info.size(); i++){
        out << ch.connections_info[i];
        write_index_data(ch.connections_info[i], in, out);
    }
}

void writingBag(std::vector<Chunk> chunks, Bag_header bh, Config conf, std::ifstream& in, std::ofstream& out){
    string f = "#ROSBAG V2.0\n";
    for(int i = 0; i < 13; i++)
        out.write(&f[i],sizeof(char));
    out << bh;
    cout << bh << endl;
    for(int i = 0; i < 4027; i++){
        char t = ' ';
        out.write(&t, 1);
    }
    int64_t size_of_file = 13 +  bh.all_size();
    for(int i = 0; i < chunks.size(); i++){
        chunks[i].info.chunk_pos = size_of_file;
        size_of_file += chunks[i].all_size();
        if(chunks[i].compression == "bz2"){
            ofstream arch(chunks[i].unarch_file + ".bz2", ios::binary);
            writing_chunk(chunks[i], in, arch, conf.unique_connections);
            string out_file;
            if(conf.correct_conditions)
                out_file = conf.out_filename_correct;
            else
                out_file = conf.out_filename_other;
            string command = "echo " + chunks[i].unarch_file + ".bz2 >> " + out_file;
            system(command.c_str());
        }
        else
            writing_chunk(chunks[i], in, out, conf.unique_connections);
    }
    for(int i = 0; i < conf.unique_connections.size(); i++){
        if(conf.taken[conf.unique_connections[i].conn]){
            out << conf.unique_connections[i];
            write_data(conf.unique_connections[i], in, out);
        }
    }
    for(int i = 0; i < chunks.size(); i++){
        out << chunks[i].info;
        write_chunk_info_data(chunks[i].info, in, out);
    }
}


void read_BAG(Config& conf, std::vector<Chunk>& chunks, Bag_header& bh, std::ifstream& in, std::ofstream& out) {
    for (int i = 0; i < 13; i++) {
        int8_t first;
        in.read((char *) &first, sizeof(int8_t));
        cout << first;
    }


    in >> bh;
    cout << bh << endl;

    while (get_op(in) == 5) {
        Chunk ch;
        in >> ch;
        cout << ch;
        if (ch.compression == "bz2") {
            cout << "We have compressed chunk!!!" << endl;
            ch.unarch_file = "chunk" + to_string(chunks.size());
            ofstream arch((ch.unarch_file + ".bz2"), ios::binary);
            write_data(ch, in, arch);
            string command = "bunzip -d " + ch.unarch_file + ".bz2";
            system(command.c_str());
            command = "rm " + ch.unarch_file + ".bz2";
            system(command.c_str());
        }
        ch.connections_to_write = conf.taken;
        in.seekg(ch.data_len, ios::cur);
        while (in && get_op(in) == 4) {
            Index_data idh;
            in >> idh;
            ch.connections_info.push_back(idh);
        }
        chunks.push_back(ch);
    }

    for (int32_t i = 0; i < bh.conn_count; i++) {
        Connection c;
        in >> c;
        conf.unique_connections[c.conn] = c;
        in.seekg(c.data_len, ios::cur);
    }

    for (int32_t i = 0; i < bh.chunk_count; i++) {
        in >> chunks[i].info;
    }
}