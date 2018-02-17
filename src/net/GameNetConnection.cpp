/* GameNetConnection.cpp:
 *     Utility class for managing communication with other players
 *     If no network communication is used, the methods of this class
 *     return trivial results.
 *     Provides useful behaviour on top of the src_net library.
 */

#include <functional>

#include <lcommon/SerializeBuffer.h>

#include <net-lib/lanarts_net.h>

#include "gamestate/GameState.h"

#include "objects/PlayerInst.h"

#include "objects/AnimatedInst.h"

#include "GameNetConnection.h"


GameNetConnection::GameNetConnection(GameState* gs) : gs(gs), _connection(NULL) {
    _message_buffer = new SerializeBuffer();
}

GameNetConnection::~GameNetConnection() {
    delete _connection;
    delete _message_buffer;
    for (int i = 0; i < _delayed_messages.size(); i++) {
        delete _delayed_messages[i].message;
    }
}

SerializeBuffer& GameNetConnection::grab_buffer(message_t type) {
    _message_buffer->clear();
    _message_buffer->write_int(type);
    return *_message_buffer;
}

void GameNetConnection::send_packet(SerializeBuffer& serializer, int receiver) {
    _connection->send_message(serializer.data(), serializer.size(), receiver);

}

static void write_or_assert_hash(SerializeBuffer& sb, unsigned int hash,
        bool is_writing) {
    if (is_writing) {
        sb.write_int(hash);
    } else {
        unsigned int val;
        sb.read(val);
        if (val != hash) {
            printf("Values 0x%X and 0x%X do not match!\n", val, hash);
            LANARTS_ASSERT(false);
        }
    }
}

static void process_level_hash(GameState* gs, GameMapState* level,
        SerializeBuffer& sb, bool isw) {
    if (level->id() == -1)
        return;

    std::vector<GameInst*> instances = level->game_inst_set().to_vector();
    for (int i = 0; i < instances.size(); i++) {
        GameInst* inst = instances[i];
        if (!dynamic_cast<AnimatedInst*>(inst)) {
            write_or_assert_hash(sb, inst->integrity_hash(), isw);
        }
    }
    //compare magic number marker
    write_or_assert_hash(sb, 0xABCDFFFF, isw);
    write_or_assert_hash(sb, level->game_inst_set().hash(), isw);
}

static void process_game_hash(GameState* gs, SerializeBuffer& sb, bool isw) {
    for (int i = 0; i < gs->game_world().number_of_levels(); i++) {
        process_level_hash(gs, gs->game_world().get_level(i), sb, isw);
    }
}
bool GameNetConnection::check_integrity(GameState* gs) {
    if (!_connection) {
        return true;
    }

    // Write out hashes
    SerializeBuffer& sb = grab_buffer(PACKET_CHECK_SYNC_INTEGRITY);
    process_game_hash(gs, sb, true);
    send_packet(sb);
    printf("Sent integrity frame %d\n", gs->frame());

    std::vector<QueuedMessage> qms = sync_on_message(
            PACKET_CHECK_SYNC_INTEGRITY);

    printf("Got %d responses\n", qms.size());
    for (int i = 0; i < qms.size(); i++) {
        // Compare hashes
        process_game_hash(gs, *qms[i].message, false);
        delete qms[i].message;
    }
    return true;
}

bool GameNetConnection::initialize_as_client(const conn_callback &callback, const char* host, int port) {
    LANARTS_ASSERT(!_connection);
    _connection = create_client_connection(host, port);
    _connection->initialize_connection(callback, 1); //1ms timeout for connection attempts
}

void GameNetConnection::initialize_as_server(const conn_callback &callback, int port) {
    LANARTS_ASSERT(!_connection);
    _connection = create_server_connection(port);
    _connection->initialize_connection(callback, 1); //1ms timeout for connection attempts
}

void GameNetConnection::set_accepting_connections(bool accept) {
    _connection->set_accepting_connections(accept);
}

void net_recv_player_actions(SerializeBuffer& sb, int sender, PlayerData& pd) {
    int frame, player_number;
    sb.read_int(frame);
    sb.read_int(player_number);

    PlayerDataEntry& pde = pd.all_players().at(player_number);

    ActionQueue actions;
    sb.read_container(actions);
    pde.action_queue.queue_actions_for_frame(actions, frame);

//    printf("Receiving %d actions for player %d from %d \n", actions.size(),
//            player_number, sender);
}
void net_send_player_actions(GameNetConnection& net, int frame,
        int player_number, const ActionQueue& actions) {
    SerializeBuffer& sb = net.grab_buffer(GameNetConnection::PACKET_ACTION);
    sb.write_int(frame);
    sb.write_int(player_number);
    sb.write_container(actions);
    net.send_packet(sb);
}

// Only the server is concerned with this message
void net_recv_connection_affirm(SerializeBuffer& sb, int sender,
        PlayerData& pd) {
    printf("Recv conn with size=%d\n", sb.size());
    std::string name;
    std::string classtype;
    sb.read(name);
    sb.read(classtype);
    printf("connection affirm read\n");
    pd.register_player(name, NULL, classtype, LuaValue(), /* is local player */ false, sender);
    printf("now there are %d players\n", (int) pd.all_players().size());
}

void net_send_connection_affirm(GameNetConnection& net, const std::string& name,
        const std::string&  classtype) {
    SerializeBuffer& sb = net.grab_buffer(
            GameNetConnection::PACKET_CLIENT2SERV_CONNECTION_AFFIRM);
    sb.write(name);
    sb.write(classtype);
    printf("connection affirm sent\n");
    net.send_packet(sb, NetConnection::SERVER_RECEIVER);
}

void net_recv_game_init_data(SerializeBuffer& sb, int sender,
        GameStateInitData& init_data, PlayerData& pd) {
    //Write seed
    sb.read(init_data);
    init_data.received_init_data = true;

    //Read player data
    int localidx;
    sb.read_int(localidx);
    int playern;
    sb.read_int(playern);
    LANARTS_ASSERT(pd.all_players().empty());
    for (int i = 0; i < playern; i++) {
        std::string name;
        std::string classtype;
        int net_id;
        sb.read(name);
        sb.read(classtype);
        sb.read_int(net_id);
        pd.register_player(name, NULL, classtype, LuaValue(), (i == localidx), net_id);
    }

    printf(
            "Received init packet: seed = 0x%X, localid = %d, nplayers = %d, "
                    "frame_action_repeat = %d, regen_level_on_death = %d, network_debug_mode = %d, time_per_step = %f\n",
            init_data.seed, localidx, playern, init_data.frame_action_repeat,
            (int) init_data.regen_on_death,
            (int) init_data.network_debug_mode, init_data.time_per_step);
}
void net_send_game_init_data(GameNetConnection& net, PlayerData& pd,
        GameStateInitData& init_data) {
    printf("net_send_game_init_data: Sending to %d players\n", pd.all_players().size());
    for (int n = 0; n < pd.all_players().size(); n++) {
        int net_id = pd.all_players()[n].net_id;
        printf("net_send_game_init_data: Pre-sending to player %d with net-id %d\n", n, net_id);

        if (net_id == 0) {
            continue; // Don't send to self
        }

        // Send a version to each player detailing which player entry is theirs
        // This is a necessary (slight) evil

        SerializeBuffer& sb = net.grab_buffer(
                GameNetConnection::PACKET_SERV2CLIENT_INITIALPLAYERDATA);

        sb.write(init_data);
        // Send which index is local player to recipient:
        sb.write_int(n);
        std::vector<PlayerDataEntry>& plist = pd.all_players();
        sb.write_int(plist.size());
        for (int i = 0; i < plist.size(); i++) {
            PlayerDataEntry& pde = plist[i];
            sb.write(pde.player_name);
            sb.write(pde.classtype);
            sb.write_int(pde.net_id);
        }
        printf("net_send_game_init_data: Sending to player %d\n", n);
        net.send_packet(sb, net_id);
    }
}
static void net_sync_and_discard(GameNetConnection& net,
        GameNetConnection::message_t) {
    std::vector<QueuedMessage> qms = net.sync_on_message(
            GameNetConnection::PACKET_SYNC_ACK);
    for (int i = 0; i < qms.size(); i++) {
        delete qms[i].message;
    }
}

static void post_sync(GameState* gs) {
    std::vector<PlayerDataEntry>& pdes = gs->player_data().all_players();
    for (int i = 0; i < pdes.size(); i++) {
        PlayerDataEntry& pde = pdes[i];
        pde.action_queue.clear();
        pde.action_queue.queue_actions_for_frame(ActionQueue(), gs->frame());
        pde.player()->actions_set() = true;
        gs->for_screens([&](){
            if (pde.player()->is_focus_player(gs)) {
                gs->view().sharp_center_on(pde.player()->ipos());
            }
        });
    }
}

void net_send_state_and_sync(GameNetConnection& net, GameState* gs) {
    if (gs->game_settings().network_debug_mode) {
        // net.check_integrity(gs);
        return;
    }
    printf("Sent sync on frame %d\n", gs->frame());
    SerializeBuffer& sb = net.grab_buffer(GameNetConnection::PACKET_FORCE_SYNC);
    if (!net.is_connected())
        return;
    // Make sure we don't receive any stray actions after sync.
//    gs->local_player()->enqueue_io_actions(gs);
//    players_poll_for_actions(gs);

    // Make sure we are all sync'd with the same rng, even if we have to contrive a state.
    int mtwistseed = gs->rng().rand();
    gs->rng().init_genrand(mtwistseed);
    sb.write_int(mtwistseed);

    gs->serialize(sb);
    net.send_packet(sb, NetConnection::ALL_RECEIVERS);

    // Wait for clients to receive the synch data before continuing
    net_sync_and_discard(net, GameNetConnection::PACKET_SYNC_ACK);
    post_sync(gs);
    net_send_sync_ack(net);
}

void net_recv_sync_data(SerializeBuffer& sb, GameState* gs) {
    printf("Got sync on frame %d\n", gs->frame());
    int mtwistseed;
    sb.read_int(mtwistseed);
    gs->rng().init_genrand(mtwistseed);
    gs->deserialize(sb);
    std::vector<PlayerDataEntry>& pdes = gs->player_data().all_players();
    for (int i = 0; i < pdes.size(); i++) {
        pdes[i].player()->set_local_player(pdes[i].is_local_player);
    }
}

void net_send_sync_ack(GameNetConnection& net) {
    SerializeBuffer& sb = net.grab_buffer(GameNetConnection::PACKET_SYNC_ACK);
    net.send_packet(sb);
}

static int find_message_type(QueuedMessage& qm,
        std::vector<QueuedMessage>& delayed_messages, int type) {
    for (int i = 0; i < delayed_messages.size(); i++) {
        qm = delayed_messages[i];
        SerializeBuffer* msg = qm.message;
        int msg_type;
        msg->read_int(msg_type);
        msg->move_read_position(-(int) sizeof(int));
        if (type == msg_type) {
            return i;
        }
    }
    return -1;
}

static bool extract_message_type(QueuedMessage& qm,
        std::vector<QueuedMessage>& delayed_messages, int type) {
    int location = find_message_type(qm, delayed_messages, type);
    if (location != -1) {
        qm.message->move_read_position(sizeof(int));
        delayed_messages.erase(delayed_messages.begin() + location);
        return true;
    }
    return false;
}

static bool has_message(std::vector<QueuedMessage>& delayed_messages,
        int type) {
    for (int i = 0; i < delayed_messages.size(); i++) {
        SerializeBuffer* msg = delayed_messages[i].message;
        int msg_type;
        msg->read_int(msg_type);
        msg->move_read_position(-(int) sizeof(int));
        if (type == msg_type) {
            return true;
        }
    }
    return false;
}

bool GameNetConnection::consume_sync_messages(GameState* gs) {
//    printf("Delayed Messages: %d\n", _delayed_messages.size());
    QueuedMessage qm;
    if (!extract_message_type(qm, _delayed_messages, PACKET_FORCE_SYNC)) {
        return false;
    }
    // Make sure we don't receive any stray actions after sync.
//    gs->local_player()->enqueue_io_actions(gs);
//    players_poll_for_actions(gs);

    printf("Found sync buffer!\n");
    net_recv_sync_data(*qm.message, gs);
    delete qm.message;

    net_send_sync_ack(*this);
    post_sync(gs);
    // Wait for server to receive acks for all clients before continuing
    net_sync_and_discard(*this, PACKET_SYNC_ACK);

    return true;
}

void net_send_chatmessage(GameNetConnection& net, ChatMessage & message) {
    if (!net.connection()) {
        return;
    }
    SerializeBuffer& sb = net.grab_buffer(
            GameNetConnection::PACKET_CHAT_MESSAGE);
    message.serialize(sb);
    net.send_packet(sb);
}

bool GameNetConnection::has_incoming_sync() {
    return has_message(_delayed_messages, PACKET_FORCE_SYNC);
}

void GameNetConnection::_queue_message(SerializeBuffer* serializer,
        int receiver) {
    _delayed_messages.push_back(QueuedMessage(serializer, receiver));
}

bool GameNetConnection::_handle_message(int sender,
        SerializeBuffer& serializer) {
    // Read type from buffer
    message_t type;
    serializer.read_int(type);

    switch (type) {

    case PACKET_CLIENT2SERV_CONNECTION_AFFIRM: {
        net_recv_connection_affirm(serializer, sender, gs->player_data());
        break;
    }
    case PACKET_SERV2CLIENT_INITIALPLAYERDATA: {
        net_recv_game_init_data(serializer, sender, gs->game_state_init_data(), gs->player_data());
        break;
    }

    case PACKET_ACTION: {
        net_recv_player_actions(serializer, sender, gs->player_data());
        break;
    }
    case PACKET_CHAT_MESSAGE: {
        ChatMessage msg;
        msg.deserialize(serializer);
        gs->for_screens([&]() {
            gs->game_chat().add_message(msg);
        });
        break;
    }
    default:
        serializer.move_read_position(-(int) sizeof(int));
        return false;
    }
    return true;
}

static void fill_msg_buffer(SerializeBuffer* sb, const char* msg, size_t len) {
    sb->clear();
    sb->write_raw(msg, len);
}
void GameNetConnection::_message_callback(int sender, const char* msg,
        size_t len, bool queue_messages) {
    message_t type;

    // Fill buffer with message
    fill_msg_buffer(_message_buffer, msg, len);

    if (queue_messages || !_handle_message(sender, *_message_buffer)) {
        _delayed_messages.push_back(QueuedMessage(_message_buffer, sender));
        _message_buffer = new SerializeBuffer();
    }
}

static void gamenetconnection_consume_message(receiver_t sender, void* context,
        const char* msg, size_t len) {
    ((GameNetConnection*) context)->_message_callback(sender, msg, len, false);
}

int GameNetConnection::poll_messages(int timeout) {
    if (_connection) {
        for (int i = 0; i < _delayed_messages.size(); i++) {
            QueuedMessage& qm = _delayed_messages[i];
            if (_handle_message(qm.sender, *qm.message)) {
                delete qm.message;
                _delayed_messages.erase(_delayed_messages.begin() + i);
                i--;
            }
        }
        perf_timer_begin("*** NETWORK POLLING ***");
        int polln = _connection->poll(gamenetconnection_consume_message, (void*) this,
                timeout);
        perf_timer_end("*** NETWORK POLLING ***");
        return polln;
    }
    return 0;
}

static void gamenetconnection_queue_message(receiver_t sender, void* context,
        const char* msg, size_t len) {
    ((GameNetConnection*) context)->_message_callback(sender, msg, len, true);
}

std::vector<QueuedMessage> GameNetConnection::sync_on_message(message_t msg) {
    std::vector<QueuedMessage> responses;
    if (!_connection) {
        return responses;
    }
    QueuedMessage qm;
    const int timeout = 1;
    gs->for_screens([&]() {
        PlayerDataEntry& pde = gs->local_player_data();
        std::vector<bool> received(gs->player_data().all_players().size(), false);

        bool all_ack = false;
        while (!all_ack) {
            _connection->poll(gamenetconnection_queue_message, (void*) this,
                    timeout);

            int idx;
            while ((idx = find_message_type(qm, _delayed_messages, msg)) != -1) {
                if (received[qm.sender]) {
                    break;
                }
                received[qm.sender] = true;
                qm.message->move_read_position(sizeof(int));
                responses.push_back(qm);
                _delayed_messages.erase(_delayed_messages.begin() + idx);
            }

            all_ack = true;
            for (int i = 0; i < received.size(); i++) {
                if (!received[i] && i != pde.net_id) {
                    all_ack = false;
                    break;
                }
            }
        }
    });
    return responses;
}
