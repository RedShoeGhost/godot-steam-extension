extends Steam

const UNIQUE_KEY = "demo1234"
const UNIQUE_VALUE = "c21hbGwxMjM0"

func _ready():
	var init = init()
	if init:
		lobby_created.connect(_on_lobby_created)
		lobby_match_list.connect(_on_lobby_match_list)
		
		add_request_lobby_list_distance_filter(3)
		add_request_lobby_list_string_filter(UNIQUE_KEY, UNIQUE_VALUE, 0)
		request_lobby_list()
	else:
		print("Steam is not running!")

func _process(_delta):
	run_callbacks()

func _on_lobby_created(result: int, lobby_id: int):
	set_lobby_data(lobby_id, UNIQUE_KEY, UNIQUE_VALUE)

func _on_lobby_match_list(lobbies: Array):
	if lobbies.size() > 0:
		join_lobby(lobbies[0])
	else:
		create_lobby(2, 4)

