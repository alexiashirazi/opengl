#version 410 core

// Intrări din vertex shader
in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

// Ieșire finală a shader-ului
out vec4 fColor;

// Uniforme pentru iluminare
uniform vec3 lightDir;
uniform vec3 lightColor;

// Uniforme pentru texturi
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Parametri pentru iluminare
vec3 ambient;
float ambientStrength = 0.1f; // Intensitatea iluminării ambientale
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

//pentru point light
uniform vec3 pointLightPos;
uniform vec3 pointLightColor;
uniform float pointLightConstant;
uniform float pointLightLinear;
uniform float pointLightQuadratic;
uniform bool enabledPL;
uniform mat4 view; // Matricea view transmisă din codul C++



///ceata
uniform vec3 fogColor;     // Culoarea ceții
uniform float fogDensity;  // Densitatea ceții



//umbre procentage closer filtering
float computeShadowPCF() {
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5f + 0.5f; // Conversie la coordonate [0, 1]

    //verificam daca bate lumina
    if (normalizedCoords.z > 1.0) {
        return 0.0;
    }
   //bias ne ajuta sa prevenim shadow acne si il calculam in functie de unghiul de incidenta
    
    vec3 normalEye = normalize(fNormal);
    vec3 lightDirN = normalize(lightDir);
    float bias = max(0.05 * (1.0 - dot(normalEye, lightDirN)), 0.005);//unghiul devine mai mic cand lumina este aproape perpendiculara pe suprafata.

    float shadow = 0.0;
    float texelSize = 1.0 / 2048.0; //rezolutia din shadow map

    //luam texeii adiacenti
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(shadowMap, normalizedCoords.xy + offset).r;
            shadow += (normalizedCoords.z - bias > closestDepth) ? 1.0 : 0.0;
        }
    }

    //media umbrei
    shadow /= 9.0;
    return shadow;
}


//lumina directionala
void computeLightComponents() {		
    vec3 cameraPosEye = vec3(0.0f); // Poziția camerei în spațiul Eye
    vec3 normalEye = normalize(fNormal); // Normala normalizată
    vec3 lightDirN = normalize(lightDir); // Direcția luminii normalizată
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz); // Direcția către cameră normalizată

    //calcul iluminare ambientala
    ambient = ambientStrength * lightColor;

    //calcul iluminare difuza
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //calcul iluminare speculara
    vec3 reflection = reflect(-lightDirN, normalEye); // Reflexia luminii
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}

//pointLight
void computePointLight(inout vec3 ambient, inout vec3 diffuse, inout vec3 specular, vec3 pointPos, vec3 pointColor, float constant, float linear, float quadratic)
{
     if (!enabledPL) { 
	return; 
    }

   vec3 pointLightPosEye = vec3(view * vec4(pointLightPos, 1.0));	
    //vec3 fPosEye = vec3(0.0);
    vec3 normalEye = normalize( fNormal);
    vec3 lightDirEye = normalize(pointLightPosEye  - fPosEye.xyz);

    float distance = length(pointLightPosEye  - fPosEye.xyz);

    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance*distance));

    vec3 viewDir = normalize(-fPosEye.xyz);
    ambient += ambientStrength * pointColor * attenuation;

    float diff = max(dot(normalEye, lightDirEye), 0.0);
    diffuse += diff * pointColor * attenuation;

    vec3 reflectDir = reflect(-lightDirEye, normalEye);
    float s = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    specular += specularStrength * s * pointColor * attenuation;

}
float computeFogFactor() {
    float distance = length(fPosEye.xyz); // Distanța fragmentului față de cameră
    float fogFactor = exp(-pow(distance * fogDensity, 2.0)); // Ceață exponențială pătrată
    return clamp(fogFactor, 0.0, 1.0); // Constrânge valoarea între [0, 1]
}



// Funcția principală
void main() {
    // Calcul iluminare
    computeLightComponents();

    // Culori de bază (modulate cu texturi)
    vec3 baseColor = vec3(0.9f, 0.35f, 0.0f); // Culoare implicită (fallback)
    ambient *= texture(diffuseTexture, fTexCoords).rgb;
    diffuse *= texture(diffuseTexture, fTexCoords).rgb;
    specular *= texture(specularTexture, fTexCoords).rgb;

    // Calcul umbre folosind PCF
    float shadow = computeShadowPCF();
    float shadowIntensity = 0.7; // Intensitatea umbrei
    computePointLight(ambient, diffuse, specular, pointLightPos, pointLightColor, pointLightConstant, pointLightLinear, pointLightQuadratic);




    // Combinarea componentelor de iluminare și aplicarea umbrei
    vec3 color = min((ambient + (1.0f - shadow * shadowIntensity) * diffuse) 
                     + (1.0f - shadow * shadowIntensity) * specular, 1.0f);
    // Calcul factor ceață
    float fogFactor = computeFogFactor();

    // Amestecare culoare obiect cu ceață
    vec3 finalColor = mix(fogColor, color, fogFactor);
	color = finalColor;

    // Setarea culorii finale
    fColor = vec4(color, 1.0f);
}
