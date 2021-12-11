myid = 99999;
range=5;
myid = id;
myx = 0;
myy = 0;
nowx=0;
nowy=0;
mylevel = 0;
mymaxhp=0;
myexp = 0;
mydamege = 0;
mytype = 0;

function initializMonster(id, x, y, level, hp, exp, damege, type)
	myid = id;
	myx = x;
	myy = y;
	mylevel = level;
	mymaxhp=hp;
	myexp = exp;
	mydamege = damege;
	mytype = type;
end



function event_agro_fallow( player )
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   nowx = API_get_x(myid);
   nowy = API_get_y(myid);
   if(player_x <= nowx +math.floor(range/2) and player_x > nowx -math.floor(range/2)) then
		if(player_y <= nowy + math.floor(range/2) and player_y > nowy -math.floor(range/2)) then
        API_recognize_player(myid,player);
		end
	end
end