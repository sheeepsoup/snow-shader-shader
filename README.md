# snow-shader-shader
这是一个用GLSL编写的雪地shader,原理就是噪声叠加+SDF+raymarching
<img width="800" height="600" alt="1fcb9f4cd20d8df652f61de8af711e42" src="https://github.com/user-attachments/assets/0a4f52a9-9fed-47a3-8ca6-3cb4dc564b97" />
<img width="800" height="600" alt="d69592eab3c853f4fa76508fe02f0b36" src="https://github.com/user-attachments/assets/74079f17-7f3d-4513-b9b4-43ad8466661a" />
由于该代码是我高二时期编写,所以数学计算均由ai提供,但是原理和大部分代码和思路非ai 
整体效果还可,可以修改噪声图片成cpu里的噪声效果可能更佳 [由于图片噪声有分辨率限制] (主要我当时迷上纯shader编程了,但是在gpu端生成噪声是很浪费性能的行为,所以我采用了图片噪声)

图形 API：OpenGL 3.3 + GLEW + SDL[用于处理移动事件]
数学库：GLM
图像加载：SDL2_image
着色器：GLSL（Vertex / Fragment Shader）
开发语言：C++

注意!!!源代码中我只放了关键代码[由于大部分都是shader]你如果想要了解原理只需要看shader即可,当然如果你需要全部代码直接打开app文件夹,里面是所有源代码和对应程序

如果你对我的雪地代码很感兴趣并且有什么不懂的话都可以私聊询问我,对应的exe文件已放入项目,可自己体验
