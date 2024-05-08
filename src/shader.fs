#version 330 core

out vec4 FragColor;
in vec3 FragPos;
in vec3 vertex_color;
in vec3 vertex_normal;

struct Transform {
	mat4 model;
	mat4 view;
	mat4 projection;
};
uniform Transform trans;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
uniform Material mat;

struct DirectionalLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirectionalLight dir_light;

struct PointLight {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float attenuation_coefficient[3];
};
uniform PointLight point_light;

struct SpotLight {
	vec3 position;
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float attenuation_coefficient[3];
	float cutoff_angle;
	float exponent;
};
uniform SpotLight spot_light;

uniform vec3 camera_pos;
uniform int light_mode;
uniform int per_fragment;

vec4 directionalLighting() {
	vec3 view_light_dir;
	vec3 view_camera_pos;
	vec3 N, L, V, R, ambient, diffuse, specular;

	view_light_dir = vec3(transpose(inverse(trans.view)) * vec4(dir_light.direction, 0.0));
	view_camera_pos = vec3(trans.view * vec4(camera_pos, 1.0));

	N = normalize(vertex_normal);
	L = -normalize(view_light_dir);
	V = normalize(FragPos - view_camera_pos);
	R = normalize(reflect(L, N));

	ambient = mat.ambient * dir_light.ambient;
	diffuse = mat.diffuse * dir_light.diffuse * max(dot(N, L), 0.0);
	specular = mat.specular * dir_light.specular * pow(max(dot(R, V), 0.0), mat.shininess);
	return vec4(ambient + diffuse + specular, 1.0) * vec4(vertex_color, 1.0);
}

vec4 pointLighting() {
	vec3 view_light_pos;
	vec3 view_camera_pos;
	vec3 view_light2vertex;
	float dist, attenuation;
	vec3 N, L, V, R, ambient, diffuse, specular;

	view_light_pos = vec3(trans.view * vec4(point_light.position, 1.0));
	view_camera_pos = vec3(trans.view * vec4(camera_pos, 1.0));
	view_light2vertex = FragPos - view_light_pos;

	N = normalize(vertex_normal);
	L = -normalize(view_light2vertex);
	V = normalize(FragPos - view_camera_pos);
	R = normalize(reflect(L, N));

	dist = length(view_light2vertex);
	attenuation = 1.0 / (point_light.attenuation_coefficient[0] + point_light.attenuation_coefficient[1] * dist + point_light.attenuation_coefficient[2] * dist * dist);

	ambient = mat.ambient * point_light.ambient;
	diffuse = mat.diffuse * point_light.diffuse * max(dot(N, L), 0.0) * attenuation;
	specular = mat.specular * point_light.specular * pow(max(dot(R, V), 0.0), mat.shininess) * attenuation;
	return vec4(ambient + diffuse + specular, 1.0) * vec4(vertex_color, 1.0);
}

vec4 spotLighting() {
	vec3 view_light_pos;
	vec3 view_light_dir;
	vec3 view_camera_pos;
	vec3 view_light2vertex;
	float dist, attenuation, theta, epsilon, effect;
	vec3 N, L, V, R, ambient, diffuse, specular;

	view_light_pos = vec3(trans.view * vec4(spot_light.position, 1.0));
	view_light_dir = vec3(transpose(inverse(trans.view)) * vec4(spot_light.direction, 0.0));
	view_camera_pos = vec3(trans.view * vec4(camera_pos, 1.0));
	view_light2vertex = FragPos - view_light_pos;

	N = normalize(vertex_normal);
	L = -normalize(view_light2vertex);
	V = normalize(FragPos - view_camera_pos);
	R = normalize(reflect(L, N));

	dist = length(view_light2vertex);
	attenuation = 1.0 / (spot_light.attenuation_coefficient[0] + spot_light.attenuation_coefficient[1] * dist + spot_light.attenuation_coefficient[2] * dist * dist);

	theta = dot(normalize(view_light2vertex), normalize(view_light_dir));
	epsilon = cos(radians(spot_light.cutoff_angle));
	if (theta > epsilon) {
		effect = pow(max(theta, 0.0), spot_light.exponent);
	} else {
		effect = 0.0;
	}

	ambient = mat.ambient * spot_light.ambient;
	diffuse = mat.diffuse * spot_light.diffuse * max(dot(N, L), 0.0) * attenuation * effect;
	specular = mat.specular * spot_light.specular * pow(max(dot(R, V), 0.0), mat.shininess) * attenuation * effect;
	return vec4(ambient + diffuse + specular, 1.0) * vec4(vertex_color, 1.0);
}

vec4 lighting() {
	if (light_mode == 0) {
		return directionalLighting();
	} else if (light_mode == 1) {
		return pointLighting();
	} else if (light_mode == 2) {
		return spotLighting();
	}
}

void main() {
	// [TODO]
	if (per_fragment == 0) {
		FragColor = vec4(vertex_color, 1.0);
	} else {
		FragColor = lighting();
	}
}
