#version 330 core
out vec4 FragColor;
#define SNOW_LAYERS 5          // 雪地层数
#define SNOW_ROUGHNESS 0.3     // 雪地粗糙度
#define SNOW_SOFTNESS 0.8      // 雪地柔软度
#define INTERVEL_GROUND 4.0//地面噪声的间隔取点,用于控制地形的陡峭程度
#define K 1.0//斜率系数,用于控制生成地面时的陡峭程度
#define MAX_MARCH_NUM 40//最大步进次数,用于实现光线步进的性能优化
#define NORMAL_ITERATIONS 36//法线迭代次数
#define MIN_TEST_DIST 0.01//最小的光线碰撞检测距离,用于光线步进
#define GROUND_K 3.0//取决于地面的陡峭程度
#define GROUND_SNOW_DENT_DENSITY 15.0//地面雪地的凹凸的密集程度
#define GROUND_SNOW_DENT_CONTENT 0.05//地面雪地的凹陷程度[深度]
#define EPSILON 0.01//极小偏移取高度来计算法线

#define MOUNTAIN_DEVIDEAL_MAX_HEIGH 60//最高的山的高度[噪声值=1的时候]
#define MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST 80.0//最高山峰的间距
#define MOUNTAIN_DEVIDEAL_MAX_HEIGH_JUDEAGE 0.2//剔除小于这个值噪声的山峰,避免出现明显块状出现山脉的现象
#define MOUNTAIN_DEVIDEAL_ONCE_SIGNIFICANCE_LOW 0.8//每次降低的权重[当前值*权重]
#define MOUNTAIN_DEVIDEAL_MAX_HEIGH_REFINE 2//最高的山的迭代生成次数[指下面提到只取最高点的那个最高点山峰]

uniform vec3 cameraPos;
uniform vec3 Forward;
uniform vec3 Right;
uniform vec3 Up;

uniform sampler2D snowDentText;//雪地凹凸纹理

uniform  sampler2D noiseTex; //噪声纹理


//光线方向
vec3 SunDir = vec3(1,-1,-1);

//性能选择部分****************************
bool B_NormalCaculate = false;//高精度法线计算
bool B_LightCaculate = true;//高精度光照计算
bool B_SnowGroundDigtial = false;//地面高精度计算凹陷[高精度的做法是把原来(4点构造的平滑曲线,然后直接-凹陷)改成所有的凹陷和4点一起计算平滑,不减]
//************************************
//地形系统-------------------------------------------------------------------------

// 光线与平面相交检测
float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal) { 
    return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0); 
}


//*哈希函数,获取噪声
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}
//*获取某一点的噪声
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    // 双线性插值
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}
//*获取噪声值以及带斜率的某一点噪声
vec3 noiseWithPreciseGradient(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    // 获取四个角点的噪声值
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    // 插值权重
    vec2 u = f * f * (3.0 - 2.0 * f);
    vec2 du = 6.0 * f * (1.0 - f);  // u的导数
    
    // 噪声值
    float noiseValue = mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
    
    // 计算x方向斜率（解析求导）
    float dx = mix(b - a, d - c, u.y) * du.x;
    
    // 计算y方向斜率（解析求导）
    float dy = mix(c - a, d - b, u.x) * du.y;
    
    return vec3(noiseValue, dx, dy);
}
//*fbm噪声
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < 4; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    return value;
}
//构建好的导数,用于制作平滑地面
// 输入：当前点坐标，2个零点坐标，斜率系数
// 输出：高度 x和y方向的导数值
//注意这里的P点是(x,y,当前点的噪声值)
// 修改为返回高度和导数
vec3 calculateDoubleZeroDerivative(vec2 currentPos, vec3 P1, vec3 P2, vec3 P3, vec3 P4, float k) {
    float h1 = P1.z;
    float h2 = P2.z;  
    float h3 = P3.z;
    float h4 = P4.z;
    
    // 计算当前点在网格内的局部坐标 [0,1]
    vec2 localPos = (currentPos - P1.xy) / INTERVEL_GROUND;
    vec2 u = localPos * localPos * (3.0 - 2.0 * localPos); // 平滑插值
    vec2 du = 6.0 * localPos * (1.0 - localPos); // u的导数
    
    // 计算高度
    float bottom = mix(h1, h3, u.x);
    float top = mix(h2, h4, u.x);
    float height = mix(bottom, top, u.y);
    
    // 计算导数
    float bottom_x = h3 - h1;
    float top_x = h4 - h2;
    float df_dx = mix(bottom_x, top_x, u.y) * du.x / INTERVEL_GROUND;
    
    float left_y = h2 - h1;
    float right_y = h4 - h3;
    float df_dy = mix(left_y, right_y, u.x) * du.y / INTERVEL_GROUND;
    
    return vec3(height, df_dx * k, df_dy * k); // x=高度, y=dx, z=dy
}
//获取当前坐标的地形高度 斜率x 斜率z
//输入: 当前位置
//输出: 高度 x斜率 y斜率
vec3 GetNowCatgory(vec2 pos) {
//如果是低精度计算
    if(!B_SnowGroundDigtial) {
	    vec2 newP = vec2(
                        floor(pos.x / INTERVEL_GROUND) * INTERVEL_GROUND ,
                        floor(pos.y / INTERVEL_GROUND) * INTERVEL_GROUND 
                         );//噪声里当前pos对应的间隔的取点位置,如[INTERVEL=4,p(5,5)->(4,4)]
        //插值计算当前点的坐标[这样做是为了平滑这些杂乱的点,做出小丘陵的效果]

        //顶点的4个点
        vec2 P1 = newP;
        vec2 P2 = vec2(newP.x,newP.y + INTERVEL_GROUND);
        vec2 P3 = vec2(newP.x + INTERVEL_GROUND,newP.y);
        vec2 P4 = vec2(newP.x + INTERVEL_GROUND,newP.y + INTERVEL_GROUND);

        //计算最这个区域的最小高度[插值最小]

        //↓**************************************************************************
        //获取四个角点的噪声值
        float h1 = fbm(P1);
        float h2 = fbm(P2);
        float h3 = fbm(P3);
        float h4 = fbm(P4);
    
 

        //现在开始构建一个特定函数f导(x),使得p1.x  p2.x L.x对应的导数为0(为了获得一个平滑的曲线,因此求斜率)[因为这四个点是正方形,所以p1.x==p3.x]
        //calculateTripleZeroDerivative()为构建好的导数函数
        vec3 derivatives = calculateDoubleZeroDerivative(pos, vec3(P1,h1), vec3(P2,h2)
        ,vec3(P3,h3),vec3(P4,h4), K);

        float currentHeight = derivatives.x;


           //制作凹陷[利用给的凹凸纹理的r值]
        vec2 texCoord = vec2(pos.x,pos.y) / GROUND_SNOW_DENT_DENSITY;
        float snowDent = texture(snowDentText,texCoord).r;
        currentHeight -= snowDent * GROUND_SNOW_DENT_CONTENT;
        currentHeight *=  GROUND_K;

	    return vec3(currentHeight,derivatives.yz);//返回高度 x斜率 y斜率
     }
     else{//高精度
        return vec3(0,0,0);//一会做
     }
}

//获取当前坐标的地形高度
//输入: 当前位置
//输出: 高度
float GetNowHeight(vec2 pos) {
    vec2 newP = vec2(
        floor(pos.x / INTERVEL_GROUND) * INTERVEL_GROUND,//注意这里floor固定向上取证!!!int取证会在负正的地方反向一下,导致地形崩坏!!!![排查了一周]
        floor(pos.y / INTERVEL_GROUND) * INTERVEL_GROUND
    );
    
    //顶点的4个点
    vec2 P1 = newP;
    vec2 P2 = vec2(newP.x, newP.y + INTERVEL_GROUND);
    vec2 P3 = vec2(newP.x + INTERVEL_GROUND, newP.y);
    vec2 P4 = vec2(newP.x + INTERVEL_GROUND, newP.y + INTERVEL_GROUND);

    //获取四个角点的噪声值
    float h1 = fbm(P1);
    float h2 = fbm(P2);
    float h3 = fbm(P3);
    float h4 = fbm(P4);
    
    //计算当前点在区域内的相对位置[0,1]
    vec2 localPos = (pos - newP) / INTERVEL_GROUND;
    vec2 u = localPos * localPos * (3.0 - 2.0 * localPos); // 平滑插值

  // 双线性插值计算当前点高度 - 确保正确的插值顺序
    float bottom = mix(h1, h3, u.x);  // 底部边缘 (P1-P3)
    float top = mix(h2, h4, u.x);     // 顶部边缘 (P2-P4)
    float currentHeight = mix(bottom, top, u.y);

    //制作凹陷[利用给的凹凸纹理的r值]
    vec2 texCoord = vec2(pos.x,pos.y) / GROUND_SNOW_DENT_DENSITY;
    float snowDent = texture(snowDentText,texCoord).r;
    currentHeight -= snowDent * GROUND_SNOW_DENT_CONTENT;
         
      
    return currentHeight * GROUND_K;
}


//用于判断噪声是否小于预设值,若小于直接返回0[具体可以看下面的真地形的代码,只用于下面]
float JudgeNoise(float nowNoise){
    float returnw = nowNoise;
    if(nowNoise< MOUNTAIN_DEVIDEAL_MAX_HEIGH_JUDEAGE)returnw = 0;
    return returnw;
}
//真·获取分层地形的高度
//原理核心:分多层,分权重,第一层决定高山的位置,第二层等等分高山内部的较高的小山
//对于大层而言每次分层完取最高点  最小层决定细节,直接-H
float GetRealHeigh(vec2 pos){
    //第一层,获取最高点的山峰
    float FinalHeigh = 0;//最终的高,用于比对和叠加
    for(int i = 0;i<MOUNTAIN_DEVIDEAL_MAX_HEIGH_REFINE;i++){//多次迭代,只取最高点,每次完毕都会*权重
        vec2 newP = vec2(//该点的对于噪声值的坐标
            floor(pos.x / MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST) * MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST,//注意这里floor固定向上取证!!!int取证会在负正的地方反向一下,导致地形崩坏!!!![排查了一周]
            floor(pos.y / MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST) * MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST
        );
          //顶点的4个点
        vec2 P1 = newP;
        vec2 P2 = vec2(newP.x, newP.y + MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST);
        vec2 P3 = vec2(newP.x + MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST, newP.y);
        vec2 P4 = vec2(newP.x + MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST, newP.y + MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST);

        //获取四个角点的噪声值
        float h1 = JudgeNoise(fbm(P1));//注:这里处理了4个点的噪声,若小于预设值则表面这里不要山
        float h2 = JudgeNoise(fbm(P2));
        float h3 = JudgeNoise(fbm(P3));
        float h4 = JudgeNoise(fbm(P4));
       
        

        //计算当前点在区域内的相对位置[0,1]
        vec2 localPos = (pos - newP) / MOUNTAIN_DEVIDEAL_HEIGHTEST_DIST;
        vec2 u = localPos * localPos * (3.0 - 2.0 * localPos); // 平滑插值

      // 双线性插值计算当前点高度 - 确保正确的插值顺序
        float bottom = mix(h1, h3, u.x);  // 底部边缘 (P1-P3)
        float top = mix(h2, h4, u.x);     // 顶部边缘 (P2-P4)
        //                           基础的高               最大的高                        权重                              次数
        float currentHeight = mix(bottom, top, u.y) * MOUNTAIN_DEVIDEAL_MAX_HEIGH * (MOUNTAIN_DEVIDEAL_ONCE_SIGNIFICANCE_LOW * i);
        //现在得到了当前坐标的高
        if(currentHeight > FinalHeigh)FinalHeigh = currentHeight;


    }
    

    return FinalHeigh;
}

//光线系统--------------------------------------------------------------------

// 专门用于法线计算的高度函数（更高精度）
float GetNowHeightForNormal(vec2 pos) {
    vec2 newP = vec2(
        floor(pos.x / INTERVEL_GROUND) * INTERVEL_GROUND,
        floor(pos.y / INTERVEL_GROUND) * INTERVEL_GROUND
    );
    
    vec2 P1 = newP;
    vec2 P2 = vec2(newP.x, newP.y + INTERVEL_GROUND);
    vec2 P3 = vec2(newP.x + INTERVEL_GROUND, newP.y);
    vec2 P4 = vec2(newP.x + INTERVEL_GROUND, newP.y + INTERVEL_GROUND);

    // 使用更高精度的fbm计算（类似海洋代码）
    float h1 = fbm(P1);
    float h2 = fbm(P2);
    float h3 = fbm(P3);
    float h4 = fbm(P4);
    
    // 可以在这里添加多层级fbm计算来提高精度
    for(int i = 1; i < NORMAL_ITERATIONS / 9; i++) {  // 适当减少迭代避免性能问题
        float freq = pow(2.0, float(i));
        float amp = pow(0.5, float(i));
        h1 += fbm(P1 * freq) * amp;
        h2 += fbm(P2 * freq) * amp;
        h3 += fbm(P3 * freq) * amp;
        h4 += fbm(P4 * freq) * amp;
    }
    
    vec2 localPos = (pos - newP) / INTERVEL_GROUND;
    vec2 u = localPos * localPos * (3.0 - 2.0 * localPos);

    float bottom = mix(h1, h3, u.x);
    float top = mix(h2, h4, u.x);
    float currentHeight = mix(bottom, top, u.y);

    // 雪地凹陷
    vec2 texCoord = vec2(pos.x, pos.y) / GROUND_SNOW_DENT_DENSITY;
    float snowDent = texture(snowDentText, texCoord).r;
    currentHeight -= snowDent * GROUND_SNOW_DENT_CONTENT;

    return currentHeight * GROUND_K;
}
// 修复后的法线计算
vec3 GetNormal_Digtial(vec3 pos) {
    float e = EPSILON;
    float depth = 1.0;
    
    // 使用高精度版本计算法线
    float H = GetNowHeightForNormal(pos.xz) * depth;
    vec2 ex = vec2(e, 0);
    
    vec3 right = vec3(pos.x + e, GetNowHeightForNormal(pos.xz + vec2(e, 0)) * depth, pos.z);
    vec3 left = vec3(pos.x - e, GetNowHeightForNormal(pos.xz + vec2(-e, 0)) * depth, pos.z);
    vec3 front = vec3(pos.x, GetNowHeightForNormal(pos.xz + vec2(0, e)) * depth, pos.z + e);
    vec3 back = vec3(pos.x, GetNowHeightForNormal(pos.xz + vec2(0, -e)) * depth, pos.z - e);
    
    vec3 dx = right - left;
    vec3 dz = front - back;
    
    return normalize(cross(dz, dx));
}

//低精度法线:
vec3 GetNormal_Accelerate(vec3 pos) {
    float e = EPSILON;
    
    // 更清晰的高度采样
    float h_center = GetNowHeight(pos.xz);
    float h_right = GetNowHeight(vec2(pos.x + e, pos.z));
    float h_left = GetNowHeight(vec2(pos.x - e, pos.z));
    float h_front = GetNowHeight(vec2(pos.x, pos.z + e));
    float h_back = GetNowHeight(vec2(pos.x, pos.z - e));
    
    // 使用中心点作为基准创建采样点
    vec3 right = vec3(pos.x + e, h_right, pos.z);
    vec3 left = vec3(pos.x - e, h_left, pos.z);
    vec3 front = vec3(pos.x, h_front, pos.z + e);
    vec3 back = vec3(pos.x, h_back, pos.z - e);
    vec3 center = vec3(pos.x, h_center, pos.z);
    
    // 计算切向量（从中心指向各方向）
    vec3 dx = right - left;
    vec3 dz = front - back;
    
    // 确保法线朝上
    vec3 normal = normalize(cross(dz, dx));
    
    // 调试：检查法线分量
    // if(normal.y < 0.9) normal = vec3(1,0,0); // 非法线显示为红色
    
    return normal;
}
//计算光照[低精度]
//当前点的位置  法线   视线方向
vec3 CaculateDirLight(vec2 pos,vec3 N,vec3 viewDir) {
  // 标准化光源方向
    vec3 lightDir = normalize(SunDir);
    
    // 环境光 - 使用天空蓝色
    vec3 ambient = vec3(0.4, 0.6, 0.8) * 0.3;

    // 漫反射 - 更自然的光照
    float diffuseIntensity = max(0.0, dot(N, -lightDir));
    vec3 diffuse = vec3(0.95, 0.97, 1.0) * diffuseIntensity;
  
    // 高光反射 - 雪地应该有较强高光
    vec3 reflectDir = reflect(lightDir, N);
    float specularIntensity = pow(max(0.0, dot(viewDir, reflectDir)), 128.0);
    vec3 specular = vec3(1.0) * specularIntensity * 0.8;

    // 距离衰减（可选）
  // 修复：使用相对于相机的位置来计算距离
    vec3 worldPos = vec3(pos.x, GetNowHeight(pos), pos.y);
    float dist = length(worldPos - cameraPos);
    float attenuation = 1.0 / (1.0 + dist * 0.1);
   
    return (ambient + (diffuse + specular) * attenuation);

}


//高精度计算光照[高精度]
float GetSnowHeight(vec2 pos) {
    vec2 newP = vec2(
        floor(pos.x / INTERVEL_GROUND) * INTERVEL_GROUND,
        floor(pos.y / INTERVEL_GROUND) * INTERVEL_GROUND
    );
    
    vec2 P1 = newP;
    vec2 P2 = vec2(newP.x, newP.y + INTERVEL_GROUND);
    vec2 P3 = vec2(newP.x + INTERVEL_GROUND, newP.y);
    vec2 P4 = vec2(newP.x + INTERVEL_GROUND, newP.y + INTERVEL_GROUND);

    // 基础地形
    float h1 = fbm(P1);
    float h2 = fbm(P2);
    float h3 = fbm(P3);
    float h4 = fbm(P4);
    
    // 添加多层雪地噪声 - 创造蓬松感
    float snowDetail = 0.0;
    float amplitude = 0.5;
    float frequency = 2.0;
    
    for(int i = 0; i < SNOW_LAYERS; i++) {
        snowDetail += amplitude * fbm(pos * frequency + vec2(i * 123.4, i * 456.7));
        amplitude *= SNOW_ROUGHNESS;
        frequency *= 1.8;
    }
    
    vec2 localPos = (pos - newP) / INTERVEL_GROUND;
    vec2 u = localPos * localPos * (3.0 - 2.0 * localPos);

    float bottom = mix(h1, h3, u.x);
    float top = mix(h2, h4, u.x);
    float currentHeight = mix(bottom, top, u.y);

    // 雪地细节 - 使用更柔和的噪声
    currentHeight += snowDetail * 0.3;

    // 雪地凹陷 - 减小凹陷深度让雪看起来更蓬松
    vec2 texCoord = vec2(pos.x, pos.y) / GROUND_SNOW_DENT_DENSITY;
    float snowDent = texture(snowDentText, texCoord).r;
    currentHeight -= snowDent * GROUND_SNOW_DENT_CONTENT * SNOW_SOFTNESS;

    return currentHeight * GROUND_K;
}
vec3 CaculateSnowLight(vec2 pos, vec3 N, vec3 viewDir) {
    vec3 lightDir = normalize(SunDir);
    
    // 更柔和的天空环境光
    vec3 ambient = mix(vec3(0.4, 0.6, 0.8), vec3(0.8, 0.9, 1.0), 0.3) * 0.4;

    // 漫反射 - 使用更宽的衰减创造蓬松感
    float diffuseIntensity = max(0.0, dot(N, -lightDir));
    diffuseIntensity = pow(diffuseIntensity, 0.7); // 软化阴影边缘
    
    vec3 diffuse = vec3(0.98, 0.99, 1.0) * diffuseIntensity;

    // 次表面散射 - 创造蓬松感的关键！
    float sss = pow(max(0.0, dot(N, lightDir)) * 0.5 + 0.5, 2.0) * 0.3;
    vec3 subsurface = vec3(0.8, 0.9, 1.0) * sss;

    // 高光反射 - 雪地应该有柔和的高光
    vec3 reflectDir = reflect(lightDir, N);
    float specularIntensity = pow(max(0.0, dot(viewDir, reflectDir)), 64.0);
    specularIntensity *= 0.6; // 降低高光强度
    vec3 specular = vec3(1.0) * specularIntensity * 0.4;

    // 距离衰减
    vec3 worldPos = vec3(pos.x, GetSnowHeight(pos), pos.y);
    float dist = length(worldPos - cameraPos);
    float attenuation = 1.0 / (1.0 + dist * 0.05); // 更柔和的衰减

    // 组合光照
    vec3 finalColor = ambient + (diffuse + specular + subsurface) * attenuation;
    
    // 添加微妙的色调变化
    float snowVariation = fbm(pos * 0.1) * 0.1;
    finalColor = mix(finalColor, vec3(0.95, 0.97, 1.0), snowVariation);
    
    return finalColor;
}
// 雪地微细节函数
vec3 AddSnowGranularity(vec3 color, vec2 pos, vec3 N, vec3 viewDir) {
    // 添加微小的雪粒细节
    float grain = fbm(pos * 50.0) * 0.1;
    
    // 基于法线和视角的颗粒强度
    float grainIntensity = (1.0 - abs(dot(N, viewDir))) * 0.3;
    grain *= grainIntensity;
    
    return color + vec3(grain * 0.1);
}





//高山


void main(){
	vec2 iRect = vec2(800,600);//窗口平面
    vec3 finalColor = vec3(1.0);//最终颜色

	//获取光线ray
	vec2 uv = (gl_FragCoord.xy - iRect.xy * 0.5) / iRect.y;//uv
	vec3 ray = normalize(Forward + Right * uv.x - Up * uv.y);
  //  vec3 TEMP_cameraPos = cameraPos;//存储一下相机位置
 
     float groundPlaneDist = intersectPlane(cameraPos, ray, vec3(0,0,0), vec3(0,1,0));
    vec3 TEMP_cameraPos = cameraPos ; // 从稍前位置开始

    float dist = 0.0;//摄像机到碰撞点的距离
    bool hitGround = false;//是否击中地面
    //RayMarching前进,绘画当前坐标
    for(int i = 0;i<MAX_MARCH_NUM;i++) {//开始步进
 
        float NOW_heigh = GetNowHeight(vec2(TEMP_cameraPos.xz));//获取当前坐标的高度

        if(TEMP_cameraPos.y - NOW_heigh <= MIN_TEST_DIST) {//倘若达到了最小距离,则表面碰撞了
            dist = length(cameraPos - TEMP_cameraPos);//计算距离
            finalColor = vec3(dist / 50) ;
            hitGround = true;
          
            break;
        }
        else{
            TEMP_cameraPos += normalize(ray) * max((TEMP_cameraPos.y - NOW_heigh) * 0.5, 0.1);//步进!
        }
    
    }
    if(hitGround == false){
          FragColor = vec4(vec3(0.4, 0.6, 0.8) * 0.3,1.0);
       //   return;
    }
    //TEMP_cameraPos最终代表交点
    //步进完毕,制作光照
    vec3 N ;
    if(B_NormalCaculate)N = GetNormal_Digtial(TEMP_cameraPos);//当前位置的法线
    else{N = GetNormal_Accelerate(TEMP_cameraPos);}

  
    vec3 Color;
   //光照计算
    if(B_LightCaculate){//高精度
        Color = CaculateSnowLight(TEMP_cameraPos.xz,N,ray);
        Color = AddSnowGranularity(Color, TEMP_cameraPos.xz, N, ray);// 添加雪地颗粒感
    }
    else{//低精度
        Color = CaculateDirLight(TEMP_cameraPos.xz,N,ray);
    }
     vec2 texCoord = vec2(TEMP_cameraPos.x,TEMP_cameraPos.z) / GROUND_SNOW_DENT_DENSITY;

    FragColor = vec4(Color,1.0);

}