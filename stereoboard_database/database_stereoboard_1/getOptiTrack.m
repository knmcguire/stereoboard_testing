clear all
close all
track = 22;


%% load image timing data
time=fopen(['take' num2str(track) '\timing.dat']);
    tline = fgets(time);

tims=textscan(time,'%f %f');
fclose(time);

t_frame=tims{1};
% remove either first or last timestamp to match number of images
t_frame = t_frame(1:end-1);


%% load optitrack data
position=fopen(['take' num2str(track) '\position.dat']);
    tline = fgets(position);

pos=textscan(position,'%f %f %f %f %f %f %f %f');
fclose(position);

% [time optishit X Y Z roll pitch yaw]
t = pos{1};
X = pos{3};
Y = pos{4};
Z = pos{5};
roll = pos{6};
pitch = pos{7};
yaw = pos{8};




%% match images to optitrack
index_frame = t_frame;
frame_number = t;
prev_index_frame = 1;

for i = 1:length(t_frame)
    
    [time_error,index_frame(i)] = min(abs(t_frame(i)-t));
    
    frame_number(prev_index_frame:index_frame(i)) = i-1;
    prev_index_frame = index_frame(i);
    
end

%% smooth data
X = smooth(X,5);
Y = smooth(Y,5);
Z = smooth(Z,5);
roll = smooth(roll,19);
pitch = smooth(pitch,19);
yaw = smooth(yaw,15);

%% estimate velocity
Vx = X(3:end)-X(1:end-2);
Vx = [0; Vx; 0];
Vy = Y(3:end)-Y(1:end-2);
Vy = [0; Vy; 0];
Vz = Z(3:end)-Z(1:end-2);
Vz = [0; Vz; 0];
yaw_rate = yaw(3:end)-yaw(1:end-2);
yaw_rate = [0; yaw_rate; 0];

cam_Vx = Vx.*cos(yaw*pi/180)-Vz.*sin(yaw*pi/180);
cam_Vz = -Vz.*cos(yaw*pi/180)-Vx.*sin(yaw*pi/180);





cam_Vx_frame = cam_Vx(index_frame); % lateral velocity (positive ->)
cam_Vz_frame = cam_Vz(index_frame); % longitudinal velocity (positive is forward)
yaw_rate_frame = yaw_rate(index_frame); % yaw rate (positive <-)


X_int = X(index_frame);
Y_int = Y(index_frame);
Z_int = Z(index_frame);
% figure(2),plot(X(10:end),'b'),hold on
% ,plot(Y(10:end),'g')
% ,plot(Z(10:end),'r')

legend xpos ypos zpos

figure(1)
subplot(3,1,1)
plot(cam_Vz_frame(10:end))
subplot(3,1,2)
plot(cam_Vx_frame(10:end))
subplot(3,1,3)
plot(yaw_rate_frame(10:end))
