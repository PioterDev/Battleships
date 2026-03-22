/**
 * Copyright © 2026 Wojciech Rączka.
 * This file is part of Battleships.
 *
 * Battleships is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Battleships is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Battleships. If not, see <https://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

static void setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

//Status defines
/*
        Status legend
        0 - new client
        1 - client in menu
        2 - created game
        3 - joining game
        4 - edited board
        5 - in edit board
        6 - waiting for opponent
        7 - match playing
        8 - match played
        */
#define status_initial 0
#define status_in_menu 1
#define status_created_game 2
#define status_joining_game 3
#define status_edited_board 4
#define status_in_edit_board 5
#define status_waiting_opponent 6
#define status_deciding_match 7
#define status_match_play 8
#define status_match_wait 9
#define status_match_display 10
#define status_match_finished 11

#define MAX_SHIP_SIZE 4

//Connect Message
constexpr std::string_view connect_message =
    "Welcome to battleships online\n"
    "1.Create a game\n"
    "2.Join game\n"
    "3.Edit board\n";
constexpr std::string_view waiting_for_opponent_message =
    "Waiting for opponent\n";

//Board Class
class Board
{
    private:
         std::vector<std::vector<char>> board_vectors;
    protected:
    public:
        Board(int size = DEFAULT_BOARD_SIZE)
        {
            board_vectors = std::vector<std::vector<char>>(size, std::vector<char>(size,'*'));
        }
        std::vector<std::vector<char>> get_vectors()
        {
            return board_vectors;
        }
        std::string get_string()
        {
            std::string curr_str;
            for(size_t i = 0; board_vectors.size() > i; i++)
            {
                for(size_t j = 0; board_vectors[0].size() > j; j++)
                {
                    curr_str += board_vectors[i][j];
                    curr_str += ' ';
                }
                curr_str += '\n';
            }
            return curr_str;
        }
        void set_vectors(std::vector<std::vector<char>>&& board_in)
        {
            board_vectors = std::move(board_in);
        }
        bool set_string(std::string string_in)
        {
            int curry = 0;
            int currx = 0;
            for(size_t i = 0; string_in.size() > i; i++)
            {
                if(string_in[i] == ' ') continue;
                else if(string_in[i] == '\n')
                {
                    curry += 1;
                    currx = 0;
                    continue;
                }
                if(curry >= board_vectors.size()) return false;
                if(currx >= board_vectors[curry].size()) return false;
                else
                {
                    board_vectors[curry][currx] = string_in[i];
                    currx += 1;
                }
            }
            return true;
        }
        bool change_square(unsigned int posx,unsigned int posy, char new_character)
        {
            if(posx >= board_vectors.size())
            {
                return false;
            }
            if(posy >= board_vectors[0].size())
            {
                return false;
            }
            board_vectors[posy][posx] = new_character;
            return true;
        }
        /*
        0 - error
        1 - shot missed
        2 - shot nailed
        */
        int shoot_board(unsigned int posx,unsigned int posy)
        {
            if(posx >= board_vectors.size())
            {
                return false;
            }
            if(posy >= board_vectors[0].size())
            {
                return false;
            }
            if(board_vectors[posy][posx] == '0') return 0;
            if(board_vectors[posy][posx] == '!') return 0;
            if(board_vectors[posy][posx] == '*'){
                board_vectors[posy][posx] = '0';
                return 1;
            }
            if(board_vectors[posy][posx] == '#'){
                board_vectors[posy][posx] = '!';
                return 2;
            }
            return 0;
        }
        void print_board()
        {
            for(size_t i = 0; board_vectors.size() > i; i++)
            {
                for(size_t j = 0; board_vectors[0].size() > j; j++)
                {
                    std::cout << board_vectors[i][j] << ' ';
                }
                std::cout << '\n';
            }
        }
        bool place_ship(int size, int posx, int posy, int orientation)
        {
            /*
            Orientation
            0 - N
            1 - E
            2 - S
            3 - W
            */
           //Check
            if(orientation > 3 || orientation < 0) return false;
           int temp_size = size;
           int temp_posx = posx;
           int temp_posy = posy;
           while(temp_size > 0)
           {
                if(temp_posy >= board_vectors.size() || temp_posy < 0) return false;
                if(temp_posx >= board_vectors[0].size() || temp_posx < 0) return false;
                if(board_vectors[temp_posy][temp_posx] != '*') return false;
                switch(orientation)
                {
                    case 0:
                        temp_posy -= 1;
                        break;
                    case 1:
                        temp_posx += 1;
                        break;
                    case 2:
                        temp_posy += 1;
                        break;
                    case 3:
                        temp_posx -= 1;
                        break;
                    default:
                        break;
                }
                temp_size -= 1;
           }
           //std::cout << "Checking passed" << std::endl;
           while(size > 0)
           {
                board_vectors[posy][posx] = '#';
                switch(orientation)
                {
                    case 0:
                        posy -= 1;
                        break;
                    case 1:
                        posx += 1;
                        break;
                    case 2:
                        posy += 1;
                        break;
                    case 3:
                        posx -= 1;
                        break;
                    default:
                        break;
                }
                size -= 1;
           }
           return true;
        }
        void clear_boats(int size = 10)
        {
            board_vectors = std::vector<std::vector<char>>(size, std::vector<char>(size,'*'));
        }
        bool check_if_win()
        {
            for(size_t i = 0; board_vectors.size() > i; i++)
            {
                for(size_t j = 0; board_vectors[i].size() > j; j++)
                {
                    if(board_vectors[i][j] == '#') return false;
                }
            }
            return true;
        }
};

//Client class
class Client
{
    private:
        struct epoll_event Epol_event = {};
        /*
        Status legend
        0 - new client
        1 - client in menu
        2 - created game
        3 - joining game
        4 - edited board
        5 - in edit board
        */
        int Status = 0;
        int Ships_aval[MAX_SHIP_SIZE] = {4,3,3,2};
        Board clients_board;
    protected:
    public:
        void set_status(int new_status)
        {
            Status = new_status;
        }
        void set_epoll_event_events(uint32_t events)
        {
            Epol_event.events = events;
        }
        void set_epoll_event_data_fd(int new_fd)
        {
            Epol_event.data.fd = new_fd;
        }
        struct epoll_event* get_epoll_event_ptr()
        {
            return &Epol_event;
        }
        int get_fd()
        {
            return Epol_event.data.fd;
        }
        int get_status()
        {
            return Status;
        }
        Board get_board_class()
        {
            return clients_board;
        }
        std::string get_board_string()
        {
           return clients_board.get_string();
        }
        std::string get_board_setup_string()
        {
            std::string temp = clients_board.get_string();
            temp += "Available ships:\n";
            temp += "----------\n";
            for(size_t i = 0; MAX_SHIP_SIZE > i; i++)
            {
                temp += "Size ";
                temp += std::to_string(i+1);
                temp += " number left ";
                temp += std::to_string(Ships_aval[i]);
                temp += '\n';
            }
            temp += "----------\n";
            return temp;
        }
        bool set_ship(const std::string& ship_instructions)
        {
#if 0
            if(ship_instructions.size() != 8) return false;
            char posx_char = ship_instructions[0];
            char posy_char = ship_instructions[2];
            char ship_size_char = ship_instructions[4];
            char orientation_char = ship_instructions[6];
            static_assert(DEFAULT_BOARD_SIZE <= 10, "too lazy to implement this properly");
            int posx = posx_char - 48;
            int posy = posy_char - 48;
            int ship_size = ship_size_char - 48;
            int orientation = orientation_char - 48;
#else
            //aight, time to implement it properly
            int posx = -1, posy = -1, ship_size = -1, orientation = -1;
            sscanf(ship_instructions.c_str(), "%d %d %d %d", &posx, &posy, &ship_size, &orientation);
#endif

            if(ship_size > MAX_SHIP_SIZE || ship_size <= 0) return false;
            if(Ships_aval[ship_size-1] <= 0) return false;
            if(clients_board.place_ship(ship_size,posx,posy,orientation))
            {
                Ships_aval[ship_size-1] -= 1;
                return true;
            }
            else return false;
        }
        void clear_board()
        {
            static_assert(MAX_SHIP_SIZE == 4, "");
            Ships_aval[0] = 4;
            Ships_aval[1] = 3;
            Ships_aval[2] = 3;
            Ships_aval[3] = 2;
            clients_board.clear_boats();
        }
};


void set_fd_player_status(int fd, std::vector<Client>& players, int status)
{
    for(Client& player : players)
    {
        if(player.get_fd() == fd)
        {
            player.set_status(status);
        }
    }
}
//Game class
class Game
{
    private:
        //std::vector<Client> players;
        std::string games_name;
        std::vector<int> players_fd;
        std::vector<Board> players_boards;
        std::vector<Board> shooting_boards;
        int curr_playing, opposing_player;
        bool change_turn;
        int game_won;
    protected:
    public:
        Game(int fd, Board player_board)
        {
            players_fd.push_back(fd);
            players_boards.push_back(player_board);
            shooting_boards.emplace_back();
            games_name = std::to_string(fd) + " game";
            curr_playing = 0;
            opposing_player = -1;
            change_turn = 0;
            game_won = -1;
        }
        std::string get_name()
        {
            return games_name;
        }
        void join_game(int fd, Board player_board)
        {
            players_fd.push_back(fd);
            players_boards.push_back(player_board);
            shooting_boards.emplace_back();
            opposing_player = 1;
        }
        bool is_open()
        {
            if(opposing_player == -1) return true;
            else return false; 
        }
        std::vector<int>& get_players_fd()
        {
            return players_fd;
        }
        /*
            error
            shot missed
            shot nailed
        */
        int shoot_board(int player_fd, const std::string& shoot_instruction)
        {
            if(player_fd != players_fd[curr_playing]) return false;
            if(shoot_instruction.size() != 4) return false;
            int posx = shoot_instruction[0] - 48;
            int posy = shoot_instruction[2] - 48;

            switch(players_boards[opposing_player].shoot_board(posx,posy)) {
                case 0:
                    return 0;
                    break;
                case 1:
                    shooting_boards[curr_playing].change_square(posx,posy, '0');
                    if(players_boards[opposing_player].check_if_win()) game_won = players_fd[curr_playing];
                    change_turn = 1;
                    return 1;
                    break;
                case 2:
                    shooting_boards[curr_playing].change_square(posx,posy,'!');
                    if(players_boards[opposing_player].check_if_win()) game_won = players_fd[curr_playing];
                    return 2;
                    break;
            }
            return 0;
        }
        std::string get_shooting_board_string(int player_fd)
        {
            std::string return_string = "Your board:\n";
            for(size_t i = 0; players_fd.size() > i; i++)
            {
                if(players_fd[i] == player_fd)
                {
                    return_string += players_boards[i].get_string() + '\n';
                    return_string += "Shooting board:\n" + shooting_boards[i].get_string() + '\n';
                    if(game_won != -1)
                    {
                        if(game_won == player_fd) return_string += "YOU WON\nSend anything to continue\n";
                        else return_string += "YOU LOST\nSend anything to continue\n";
                        return return_string;
                    }
                    else{
                        if(player_fd == players_fd[curr_playing]) return_string += "YOUR TURN\n";
                        else return_string += "WAIT\n";
                        return return_string;
                    }
                }
            }
            return "";
        }
        void update_players_status(std::vector<Client>& players, int player_fd,int set_disp)
        {
            if(set_disp == 1)
            {
                for(size_t i = 0; players_fd.size() > i; i++)
                {
                    std::cout << "UPDATE PLAYER STATUS" << players_fd[i] << "\n";
                    set_fd_player_status(players_fd[i], players, status_match_display);
                }
            } 
            //HARD CODED FOR TWO PLAYERS
            else
            {
                if(game_won != -1)
                {
                    set_fd_player_status(player_fd,players,status_match_finished);
                }
                else
                {
                    if(change_turn) {
                        if(curr_playing == 1 || curr_playing == -1)
                        {
                            curr_playing = 0;
                            opposing_player = 1;
                        }
                        else
                        {
                            curr_playing = 1;
                            opposing_player = 0;
                        }
                    }
                    if(player_fd == players_fd[curr_playing]) set_fd_player_status(players_fd[curr_playing], players, status_match_play);
                    if(player_fd == players_fd[opposing_player]) set_fd_player_status(players_fd[opposing_player], players, status_match_wait);
                    change_turn = 0;
                }
            }
        }
        bool game_exit(int player_fd)
        {
            for(size_t i = 0; players_fd.size() > i; i++)
            {
                if(players_fd[i] == player_fd)
                {
                    players_fd.erase(players_fd.begin() + i);
                    if(players_fd.size() == 0) return true;
                    else if(players_fd.size() == 1)
                    {
                        game_won = players_fd[0];
                        return false;
                    }
                    break;
                }
            }
            return false;
        }
};

static int get_game_id_from_player_fd(int player_fd, std::vector<Game>& games)
{
    for(size_t i = 0; games.size() > i; i++)
    {
        std::vector<int>& game_players = games[i].get_players_fd();
        for(size_t j = 0; game_players.size() > j; j++)
        {
            if(game_players[j] == player_fd)
            {
                return i;
            }
        }
    }
    return -1;
}
static void client_disconnect(std::vector<Client>& Clients, std::vector<Game>& Games, size_t client_id, int epoll,int game_id = -1)
{
    //TU KONTYNUOWAC -- DODAC FUNKCJE OBSLUGUJACA ROZLACZENIE KLIENTA
    if(game_id >= 0)
    {
        unsigned int gid = (unsigned int)game_id;
        Games[gid].update_players_status(Clients, Clients[client_id].get_fd(), 1);
        if(Games[gid].game_exit(Clients[client_id].get_fd()) == true)
            Games.erase(Games.begin() + gid);
    }
    epoll_ctl(epoll,EPOLL_CTL_DEL,Clients[client_id].get_fd(),NULL);
    close(Clients[client_id].get_fd());
    Clients.erase(Clients.begin() + client_id);
}


int main(int argc, char** argv)
{
    //Connection variables
    struct sockaddr_in address;
    struct epoll_event event, events[10];
    std::vector<Client> Clients;
    std::vector<Game> games;
    //memset(&address,0,sizeof address);
    int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1)
    {
        std::cout << "Failed to create epoll file descriptor\n";
        return 1;
    }

    int server = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

    int port = argc > 1 ? atoi(argv[1]) : 1101;
    if(port == 0) {
        std::cerr << "Invalid port number provided (" << argv[1] << ")\n";
        return 1;
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server,(struct sockaddr*)&address,sizeof address) < 0) {
        std::cerr << "Failed to bind: " << strerror(errno) << "\n";
        return 1;
    }
    std::cout << "Listening on port " << port << "...\n";

    //Epoll setup
    event.events = EPOLLIN;
    event.data.fd = server;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD,server,&event))
    {
        std::cout << "Failed to add file descriptor to epoll\n";
        close(epoll_fd);
        return 1;
    }
    setnonblock(server);
    listen(server, 5);

    //Game vars
    std::string message_string;
    size_t choice;
    char buf[256] = {};
    //Main loop
    while(1)
    {
        //Poll wait
        const int event_cnt = epoll_wait(epoll_fd, events, 10, 3000);
        if(event_cnt < 0) {
            switch(errno) {
            case EINTR: continue;
            default:
                std::cout << "Failed to wait: " << strerror(errno) << "\n";
                return 1;
            }
        }
        const size_t event_count = (size_t)(unsigned int)event_cnt;
        if(event_count == 0) continue;
        for(size_t i = 0; i < event_count; i++)
        {
            //server -- adds new Client
            if(events[i].data.fd == server && events[i].events & EPOLLIN)
            {
                int ClientSocket;
                while((ClientSocket = accept(server, NULL, NULL)) >= 0) {
                    setnonblock(ClientSocket);
                    Clients.emplace_back();
                    Client& client = Clients.back();
                    client.set_epoll_event_events(EPOLLIN /*| EPOLLOUT*/);
                    client.set_epoll_event_data_fd(ClientSocket);

                    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ClientSocket, client.get_epoll_event_ptr()))
                    {
                        std::cout
                            << "Failed to add file descriptor to epoll: "
                            << strerror(errno)
                            << "\n";
                        close(epoll_fd);
                        return 1;
                    }
                    write(client.get_fd(), connect_message.data(), connect_message.size());
                    client.set_status(status_in_menu);
                }
                switch(errno) {
                    case EWOULDBLOCK: break;
                    default:
                        std::cout << "Failed to accept connection: " << strerror(errno) << "\n";
                        return 1;
                }
                continue;
            }
            //ClientsSocket
            //Client incoming
            if(events[i].events & EPOLLIN)
            {
                //std::cout << "mozliwe czytanie w procesie " << events[i].data.fd << '\n';
                // char buff[256] = "";
                // read(events[i].data.fd,buff,sizeof buff);
                // std::string temp_str(buff);
                // std::cout << temp_str << '\n';
                ssize_t clientIdx_ = -1;
                for(size_t j = 0; Clients.size() > j; j++)
                {
                    if(events[i].data.fd != Clients[j].get_fd()) continue;
                    clientIdx_ = (ssize_t)j;
                    break;
                }
                if(clientIdx_ < 0 || (size_t)clientIdx_ >= Clients.size()) continue;
                const size_t clientIdx = (size_t)clientIdx_;
                Client& client = Clients[clientIdx];
                switch(client.get_status()) {
                case status_in_menu:
                    if(read(client.get_fd(), buf, sizeof buf) == 0) {
                        client_disconnect(Clients, games, clientIdx, epoll_fd, get_game_id_from_player_fd(client.get_fd(), games));
                        break;
                    }
                    std::cout << "Obtained choice " << buf;
                    switch(buf[0]) {
                    case '1':
                        games.emplace_back(client.get_fd(), client.get_board_class());
                        client.set_status(status_created_game);
                        break;
                    case '2':
                        client.set_status(status_joining_game);
                        break;
                    case '3':
                        client.set_status(status_edited_board);
                        break;
                    default:
                        client.set_status(status_initial);
                        break;
                    }
                    break;
                case status_in_edit_board:
                    if(read(client.get_fd(),buf, sizeof buf) == 0){
                        client_disconnect(Clients, games, clientIdx, epoll_fd,get_game_id_from_player_fd(client.get_fd(),games));
                        break;
                    }
                    std::cout << "Obtained board effect " << buf;
                    message_string = std::string(buf);
                    if(message_string == "clear\n")
                    {
                        client.clear_board();
                        client.set_status(status_edited_board);
                    }
                    else if(message_string == "exit\n")
                    {
                        client.set_status(status_initial);
                    }
                    else
                    {
                        client.set_ship(message_string);
                        client.set_status(status_edited_board);
                    }
                    break;
                case status_deciding_match:
                    if(read(client.get_fd(),buf, sizeof buf) == 0) {
                        client_disconnect(Clients, games, clientIdx, epoll_fd, get_game_id_from_player_fd(client.get_fd(),games));
                        break;
                    }
                    choice = strtoul(buf, NULL, 10);
                    if(games.size() > choice && choice >= 0 && games[choice].is_open()) 
                    {
                        games[choice].join_game(client.get_fd(),client.get_board_class());
                        games[choice].update_players_status(Clients, client.get_fd(),1);
                        //client.set_status(status_match_display);
                        //set_fd_player_status(games[choice].get_players_fd()[0],&Clients,status_match_display);
                    }
                    else
                    {
                        client.set_status(status_joining_game);
                    }
                    break;
                case status_match_play:
                    if(read(client.get_fd(),buf, sizeof buf) == 0) {
                        client_disconnect(Clients, games, clientIdx, epoll_fd,get_game_id_from_player_fd(client.get_fd(),games));
                        break;
                    }
                    message_string = std::string(buf);
                    choice = get_game_id_from_player_fd(client.get_fd(),games);
                    if(games[choice].shoot_board(client.get_fd(),message_string)){
                        games[choice].update_players_status(Clients, client.get_fd(),1);
                    }
                    break;
                case status_match_finished:
                    choice = get_game_id_from_player_fd(client.get_fd(),games);
                    if(games[choice].game_exit(client.get_fd()) == true) games.erase(games.begin() + choice);
                    if(read(client.get_fd(),buf, sizeof buf) == 0) {
                        client_disconnect(Clients, games, clientIdx, epoll_fd, get_game_id_from_player_fd(client.get_fd(),games));
                        break;
                    }
                    client.set_status(status_initial);
                    break;
                default:
                    if(read(client.get_fd(),buf, sizeof buf) == 0) {
                        client_disconnect(Clients, games, clientIdx, epoll_fd, get_game_id_from_player_fd(client.get_fd(),games));
                        break;
                    }
                    break;
                }
            }
            //Client outgoing
            // if(events[i].events & EPOLLOUT)
        }
        for(Client& client : Clients) {
            //std::cout << "mozliwe pisanie w procesie " << events[i].data.fd << '\n';
            // ssize_t clientIdx_ = -1;
            // for(size_t j = 0; Clients.size() > j; j++)
            // {
            //     if(events[i].data.fd != Clients[j].get_fd()) continue;
            //     clientIdx_ = (ssize_t)j;
            //     break;
            // }
            // if(clientIdx_ < 0 || (size_t)clientIdx_ >= Clients.size()) continue;
            // Client& client = Clients[(size_t)clientIdx_];
            switch(client.get_status()) {
            case status_initial:
                write(client.get_fd(),connect_message.data(),connect_message.size());
                client.set_status(status_in_menu);
                break;
            case status_edited_board:
                message_string = client.get_board_setup_string();
                write(client.get_fd(),message_string.c_str(),message_string.size());
                client.set_status(status_in_edit_board);
                break;
            case status_created_game:
                write(client.get_fd(),waiting_for_opponent_message.data(),waiting_for_opponent_message.size());
                client.set_status(status_waiting_opponent);
                break;
            case status_joining_game:
                message_string =
                    "Current games:\n"
                    "----------\n";
                for(size_t k = 0; games.size() > k; k++)
                {
                    if(!games[k].is_open()) continue;
                    message_string += std::to_string(k) + " ";
                    message_string += games[k].get_name();
                    message_string += '\n';
                }
                message_string += "----------\n";
                write(client.get_fd(),message_string.c_str(),message_string.size());
                client.set_status(status_deciding_match);
                break;
            case status_match_display:
                choice = get_game_id_from_player_fd(client.get_fd(), games);
                if(choice < 0) {
                    write(client.get_fd(), "Invalid choice\n", 15);
                    client.set_status(status_deciding_match);
                    break;
                }
                //Status update earlier to get the correct players roles
                games[choice].update_players_status(Clients, client.get_fd(),0);
                message_string = games[choice].get_shooting_board_string(client.get_fd());
                write(client.get_fd(),message_string.c_str(),message_string.size());
                break;
            default:
                break;
            }
        }
        memset(buf, 0, sizeof(buf));
    }
    if(close(epoll_fd))
    {
        std::cout << "Failed to close epoll file descriptor";
    }
    // close(ClientSocket);
    close(server);
}
