myid = 99999;
range=10;
myid = id;
myx = 0;
myy = 0;
mylevel = 0;
mymaxhp=0;
myexp = 0;
mydamege = 0;
mytype = 0;
function set_uid(x)
   myid = x;
end



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




function event_peace_follow(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_SendMessage(myid, player, "HELLO");
      end
   end
end


function event_agro_fallow( player )
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   myx = API_get_x(myid);
   myy = API_get_y(myid);
   if(player_x <= myx +range/2 and player_x > myx - range/2) then
		if(player_y <= myy + range/2 and player_y > myy - range/2) then
        API_recognize_player(myid,player);
		end
	end
end