#include <iostream>
#include <vector>
#include <string>
// #include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

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

//Connect Message
std::string_view connect_message = "Welcome to battleships online\n1.Create a game\n2.Join game\n3.Edit board\n";
std::string_view waiting_for_opponent_message = "Waiting for opponent\n";

//Board Class
class board
{
    private:
         std::vector<std::vector<char>> board_vectors;
    protected:
    public:
        board(int size = DEFAULT_BOARD_SIZE)
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
        void set_vectors(std::vector<std::vector<char>> board_in)
        {
            board_vectors = board_in;
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
           int temp_size = size;
           int temp_posx = posx;
           int temp_posy = posy;
           while(temp_size > 0)
           {
                if(temp_posy >= board_vectors.size() || temp_posy < 0) return false;
                if(temp_posx >= board_vectors[0].size() || temp_posx < 0) return false;
                if(board_vectors[posy][posx] != '*') return false;
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
class client
{
    private:
        struct epoll_event* Epol_event;
        /*
        Status legend
        0 - new client
        1 - client in menu
        2 - created game
        3 - joining game
        4 - edited board
        5 - in edit board
        */
        int Status;
        int Ships_aval[3] = {4,3,3};
        board clients_board;
    protected:
    public:
        client()
        {
            Status = 0;
        }
        client(struct epoll_event* new_epol_event)
        {
            Epol_event = new_epol_event;
            Status = 0;
        }
        void delete_epol_event()
        {
            delete(Epol_event);
        }
        void set_status(int new_status)
        {
            Status = new_status;
        }
        void set_epoll_event_ptr(struct epoll_event* new_epol_event)
        {
            delete(Epol_event);
            Epol_event = new_epol_event;
        }
        void set_epoll_event_events(uint32_t events)
        {
            if(Epol_event != NULL)
            {
                Epol_event->events = events;
            }
        }
        void set_epoll_event_data_fd(int new_fd)
        {
            if(Epol_event != NULL)
            {
                Epol_event->data.fd = new_fd;
            }
        }
        struct epoll_event* get_epoll_event_ptr()
        {
            return Epol_event;
        }
        int get_fd()
        {
            return Epol_event->data.fd;
        }
        int get_status()
        {
            return Status;
        }
        board get_board_class()
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
            for(size_t i = 0; 3 > i; i++)
            {
                temp += "Size ";
                temp += std::to_string(i+1);
                temp += " number left ";
                temp += std::to_string(Ships_aval[i]);
                temp += '\n';
            }
            return temp;
        }
        bool set_ship(std::string ship_instruciotns)
        {
            if(ship_instruciotns.size() != 8) return false;
            char posx_char = ship_instruciotns[0];
            char posy_char = ship_instruciotns[2];
            char ship_size_char = ship_instruciotns[4];
            char orientation_char = ship_instruciotns[6];

            int posx = posx_char - 48;
            int posy = posy_char - 48;
            int ship_size = ship_size_char - 48;
            int orientation = orientation_char - 48;
            
            if(ship_size > 3) return false;
            if(Ships_aval[ship_size-1] <= 0) return false;
            if(clients_board.place_ship(ship_size,posx,posy,orientation))
            {
                Ships_aval[ship_size-1] -= 1;
                return true;
            }
            else
            {
                return false;
            }
        }
        void clear_board()
        {
            Ships_aval[0] = 4;
            Ships_aval[1] = 3;
            Ships_aval[2] = 3;
            clients_board.clear_boats();
        }
        
};


void set_fd_player_status(int fd, std::vector<client> *players, int status)
{
    for(size_t i = 0; players->size() > i; i++)
    {
        if(players->at(i).get_fd() == fd)
        {
            players->at(i).set_status(status);
        }
    }
}
//Game class
class game
{
    private:
        //std::vector<client> players;
        std::string games_name;
        std::vector<int> players_fd;
        std::vector<board> players_boards;
        std::vector<board> shooting_boards;
        int curr_playing, opposing_player;
        bool change_turn;
        int game_won;
    protected:
    public:
        game(int fd, board player_board)
        {
            players_fd.push_back(fd);
            players_boards.push_back(player_board);
            shooting_boards.push_back(board());
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
        void join_game(int fd, board player_board)
        {
            players_fd.push_back(fd);
            players_boards.push_back(player_board);
            shooting_boards.push_back(board());
            opposing_player = 1;
        }
        bool is_open()
        {
            if(opposing_player == -1) return true;
            else return false; 
        }
        std::vector<int> get_players_fd()
        {
            return players_fd;
        }
        /*
            error
            shot missed
            shot nailed
        */
        int shoot_board(int player_fd, std::string shoot_instruction)
        {
            if(player_fd != players_fd[curr_playing]) return false;
            if(shoot_instruction.size() != 4) return false;
            int posx = shoot_instruction[0] - 48;
            int posy = shoot_instruction[2] - 48;

            switch(players_boards[opposing_player].shoot_board(posx,posy)){
                case 0:
                    return 0;
                    break;
                case 1:
                    shooting_boards[curr_playing].change_square(posx,posy,'0');
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
        void update_players_status(std::vector<client> *players, int player_fd,int set_disp)
        {
            if(set_disp == 1)
            {
                for(size_t i = 0; players_fd.size() > i; i++)
                {
                    set_fd_player_status(players_fd[i],players,status_match_display);
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
                    if(change_turn){
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
                    if(player_fd == players_fd[curr_playing]) set_fd_player_status(players_fd[curr_playing],players,status_match_play);
                    if(player_fd == players_fd[opposing_player]) set_fd_player_status(players_fd[opposing_player],players,status_match_wait);
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

int get_game_id_from_player_fd(int player_fd, std::vector<game> games)
{
    std::vector<int> game_players;
    for(size_t i = 0; games.size() > i; i++)
    {
        game_players = games[i].get_players_fd();
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
void client_disconnect(std::vector<client> *Clients,std::vector<game> *Games, int client_id, int epoll,int game_id = -1)
{
    //TU KONTYNUOWAC -- DODAC FUNKCJE OBSLUGUJACA ROZLACZENIE KLIENTA
    if(game_id != -1)
    {
        Games->at(game_id).update_players_status(Clients,Clients->at(client_id).get_fd(),1);
        if(Games->at(game_id).game_exit(Clients->at(client_id).get_fd()) == true) Games->erase(Games->begin() + game_id);
    }
    epoll_ctl(epoll,EPOLL_CTL_DEL,Clients->at(client_id).get_fd(),NULL);
    Clients->at(client_id).delete_epol_event();
    close(Clients->at(client_id).get_fd());
    Clients->erase(Clients->begin()+client_id);
}


int main()
{
    //Connection variables
    struct sockaddr_in address;
    struct epoll_event event, events[10];
    std::vector<client> Clients;
    std::vector<game> games;
    //memset(&address,0,sizeof address);
    int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1)
    {
        std::cout << "Failed to create epoll file descriptor\n";
        return 1;
    }

    int ServerSocket = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);

    address.sin_family = AF_INET;
    address.sin_port = htons(1101);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(ServerSocket,(struct sockaddr*)&address,sizeof address);

    //Epoll setup
    event.events = EPOLLIN;
    event.data.fd = ServerSocket;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD,ServerSocket,&event))
    {
        std::cout << "Failed to add file descriptor to epoll\n";
        close(epoll_fd);
        return 1;
    }

    listen(ServerSocket,5);
    int event_count;
    //Main loop
    while(1)
    {
        //Poll wait
        event_count = epoll_wait(epoll_fd, events, 10, 3000);
        for(size_t i = 0; i < event_count; i++)
        {
            //ServerSocket -- adds new Client
            if(events[i].data.fd == ServerSocket && events[i].events & EPOLLIN)
            {
                int ClientSocket = accept(ServerSocket,NULL,NULL);
                client new_client(new struct epoll_event);
                Clients.push_back(new_client);
                size_t curr_client_num = Clients.size() - 1;
                Clients[curr_client_num].set_epoll_event_events(EPOLLIN | EPOLLOUT);
                Clients[curr_client_num].set_epoll_event_data_fd(ClientSocket);

                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD,ClientSocket,Clients[curr_client_num].get_epoll_event_ptr()))
                {
                    std::cout << "Failed to add file descriptor to epoll\n";
                    close(epoll_fd);
                    return 1;
                }                
            }
            //ClientsSocket
            else
            {
                //Game vars
                char stringbuffer[256] = {};
                std::string message_string;
                size_t choice;
                //Client incoming
                if(events[i].events & EPOLLIN)
                {
                    //std::cout << "mozliwe czytanie w procesie " << events[i].data.fd << '\n';
                    // char buff[256] = "";
                    // read(events[i].data.fd,buff,sizeof buff);
                    // std::string temp_str(buff);
                    // std::cout << temp_str << '\n';
                    for(size_t j = 0; Clients.size() > j; j++)
                    {
                        if(events[i].data.fd == Clients[j].get_fd())
                        {
                            switch(Clients[j].get_status())
                            {
                                case status_in_menu:
                                    if(read(Clients[j].get_fd(),stringbuffer,sizeof stringbuffer) == 0){
                                        client_disconnect(&Clients,&games,j,epoll_fd,get_game_id_from_player_fd(Clients[j].get_fd(),games));
                                        j -= 1;
                                        continue;
                                    }
                                    std::cout << "Obtained choice " << stringbuffer << '\n';
                                    switch(stringbuffer[0])
                                    {
                                        case '1': 
                                            games.push_back(game(Clients[j].get_fd(),Clients[j].get_board_class()));
                                            Clients[j].set_status(status_created_game);
                                            break;
                                        case '2':
                                            Clients[j].set_status(status_joining_game);
                                            break;
                                        case '3':
                                            Clients[j].set_status(status_edited_board);
                                            break;
                                        default:
                                            Clients[j].set_status(status_initial);
                                            break;
                                    }
                                    break;
                                case status_in_edit_board:
                                    if(read(Clients[j].get_fd(),stringbuffer, sizeof stringbuffer) == 0){
                                        client_disconnect(&Clients,&games,j,epoll_fd,get_game_id_from_player_fd(Clients[j].get_fd(),games));
                                        j -= 1;
                                        continue;
                                    }
                                    std::cout << "Obtained board effect " << stringbuffer << std::endl;
                                    message_string = std::string(stringbuffer);
                                    if(message_string == "clear\n")
                                    {
                                        Clients[j].clear_board();
                                        Clients[j].set_status(status_edited_board);
                                    }
                                    else if(message_string == "exit\n")
                                    {
                                        Clients[j].set_status(status_initial);
                                    }
                                    else
                                    {
                                        Clients[j].set_ship(message_string);
                                        Clients[j].set_status(status_edited_board);
                                    }
                                    break;
                                case status_deciding_match:
                                    if(read(Clients[j].get_fd(),stringbuffer, sizeof stringbuffer) == 0){
                                        client_disconnect(&Clients,&games,j,epoll_fd,get_game_id_from_player_fd(Clients[j].get_fd(),games));
                                        j -= 1;
                                        continue;
                                    }
                                    choice = atoi(stringbuffer);
                                    if(games.size() > choice && choice >= 0 && games[choice].is_open()) 
                                    {
                                        games[choice].join_game(Clients[j].get_fd(),Clients[j].get_board_class());
                                        games[choice].update_players_status(&Clients,Clients[j].get_fd(),1);
                                        //Clients[j].set_status(status_match_display);
                                        //set_fd_player_status(games[choice].get_players_fd()[0],&Clients,status_match_display);
                                    }
                                    else
                                    {
                                        Clients[j].set_status(status_joining_game);
                                    }
                                    break;
                                case status_match_play:
                                    if(read(Clients[j].get_fd(),stringbuffer, sizeof stringbuffer) == 0){
                                        client_disconnect(&Clients,&games,j,epoll_fd,get_game_id_from_player_fd(Clients[j].get_fd(),games));
                                        j -= 1;
                                        continue;
                                    }
                                    message_string = std::string(stringbuffer);
                                    choice = get_game_id_from_player_fd(Clients[j].get_fd(),games);
                                    if(games[choice].shoot_board(Clients[j].get_fd(),message_string)){
                                        games[choice].update_players_status(&Clients,Clients[j].get_fd(),1);
                                    }
                                    break;
                                case status_match_finished:
                                    choice = get_game_id_from_player_fd(Clients[j].get_fd(),games);
                                    if(games[choice].game_exit(Clients[j].get_fd()) == true) games.erase(games.begin() + choice);
                                    if(read(Clients[j].get_fd(),stringbuffer, sizeof stringbuffer) == 0){
                                        client_disconnect(&Clients,&games,j,epoll_fd,get_game_id_from_player_fd(Clients[j].get_fd(),games));
                                        j -= 1;
                                        continue;
                                    }
                                    Clients[j].set_status(status_initial);
                                    break;
                                default:
                                    if(read(Clients[j].get_fd(),stringbuffer, sizeof stringbuffer) == 0){
                                        client_disconnect(&Clients,&games,j,epoll_fd,get_game_id_from_player_fd(Clients[j].get_fd(),games));
                                        j -= 1;
                                        continue;
                                    }
                                    break;
                            }
                        }
                    }
                }
                //Client outgoing
                if(events[i].events & EPOLLOUT)
                {
                    //std::cout << "mozliwe pisanie w procesie " << events[i].data.fd << '\n';
                    for(size_t j = 0; Clients.size() > j; j++)
                    {
                        if(events[i].data.fd == Clients[j].get_fd())
                        {
                            switch(Clients[j].get_status())
                            {
                                case status_initial:
                                    write(Clients[j].get_fd(),connect_message.data(),connect_message.size());
                                    Clients[j].set_status(status_in_menu);
                                    break;
                                case status_edited_board:
                                    message_string = Clients[j].get_board_setup_string();
                                    write(Clients[j].get_fd(),message_string.c_str(),message_string.size());
                                    Clients[j].set_status(status_in_edit_board);
                                    break;
                                case status_created_game:
                                    write(Clients[j].get_fd(),waiting_for_opponent_message.data(),waiting_for_opponent_message.size());
                                    Clients[j].set_status(status_waiting_opponent);
                                    break;
                                case status_joining_game:
                                    message_string = "Current games:\n";
                                    for(size_t k = 0; games.size() > k; k++)
                                    {
                                        if(!games[k].is_open()) continue;
                                        message_string += std::to_string(k) + " ";
                                        message_string += games[k].get_name();
                                        message_string += '\n';
                                    }
                                    write(Clients[j].get_fd(),message_string.c_str(),message_string.size());
                                    Clients[j].set_status(status_deciding_match);
                                    break;
                                case status_match_display:
                                    choice = get_game_id_from_player_fd(Clients[j].get_fd(),games);
                                    //Status update earlier to get the correct players roles
                                    games[choice].update_players_status(&Clients,Clients[j].get_fd(),0);
                                    message_string = games[choice].get_shooting_board_string(Clients[j].get_fd());
                                    write(Clients[j].get_fd(),message_string.c_str(),message_string.size());
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }

        }
    }
    if(close(epoll_fd))
    {
        std::cout << "Failed to close epoll file descriptor";
    }
    // close(ClientSocket);
    close(ServerSocket);
}
