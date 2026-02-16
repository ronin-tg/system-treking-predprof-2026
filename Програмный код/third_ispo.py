import socket
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

UDP_IP = "0.0.0.0"
UDP_PORT = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.setblocking(False)

history_len = 50
y_accel_data = [0] * history_len
rotation_info = "Ожидание удара..."
direction_info = ""

plt.style.use('dark_background')
fig, ax = plt.subplots(figsize=(10, 6), facecolor='#2b2b2b')
fig.canvas.manager.set_window_title('Анализатор удара (Испытание №3)')

def setup_ax(target_ax):
    target_ax.set_facecolor('white')
    target_ax.grid(True, color='lightgray', linestyle='-', linewidth=0.5)
    target_ax.tick_params(colors='white')
    for spine in target_ax.spines.values():
        spine.set_color('gray')

def update(frame):
    global rotation_info, direction_info
    try:
        data, _ = sock.recvfrom(1024)
        vals = [float(x) for x in data.decode().split(',')]
        
        ay = vals[1]
        gx = vals[3]
        gy = vals[4]
        
        y_accel_data.append(ay)
        y_accel_data.pop(0)

        if abs(ay) > 1500:
            if ay > 3000: direction_info = "Направление: ПРАВО"
            elif ay < -3000: direction_info = "Направление: ЛЕВО"
            else: direction_info = "Направление: ЦЕНТР"
            
            if abs(gx) > 5000:
                rotation_info = "Вращение: ВЕРХНЕЕ/НИЖНЕЕ (Topspin/Backspin)"
            elif abs(gy) > 5000:
                rotation_info = "Вращение: БОКОВОЕ (Sidespin)"
            else:
                rotation_info = "Вращение: НЕТ"

        ax.clear()
        setup_ax(ax)
        ax.plot(y_accel_data, color='green', linewidth=1.5)
        ax.set_ylim(-16000, 16000)
        
        plt.suptitle(f"{direction_info}\n{rotation_info}", color='white', fontsize=14, fontweight='bold')
        
    except:
        pass

setup_ax(ax)
ani = FuncAnimation(fig, update, interval=30, cache_frame_data=False)
plt.tight_layout(rect=[0, 0.03, 1, 0.9])
plt.show()
