myid = 99999;

function set_uid(x)
   myid = x;
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


function event_peace_attack( player )
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if(1<=math.abs(player_x-my_x) or 1<=math.abs(player_y-my_y))
        API_send_attack(myid,player);
end