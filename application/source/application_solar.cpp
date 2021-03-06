#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <vector>
#include <algorithm>
using namespace std;



ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
{
  fillStars();
  fillOrbits();
  fillPlanets();
  fillParticles();
  
  initializeScreenQuad();
  initializeFramebuffer();

  initializeOrbits();
  initializeGeometry();
  initializeStars();
  initializeTextures();
  initializeParticles();

  initializeShaderPrograms();
}


//Fill Stars vector with floats. This only happens once, so i guess all the if-cases are okay
void ApplicationSolar::fillStars()
{
  for (int i = 0; i < 500; i++)
  { 
    //Random Values for Stars
    if (i % 4 == 0)
    {
      //Its a box, not a bubble, but meh. its oke
      float x = -1000  + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/2000)); 
      float y = -1000  + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/2000));
      float z = -1000  + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/2000));
      float euclid = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
      float min_dis_to_sun = 800;

      //Check if euclidean distance of star is bigger than 60.
      if(euclid > min_dis_to_sun)
      {
        all_stars.insert(std::end(all_stars), x);
        all_stars.insert(std::end(all_stars), y);
        all_stars.insert(std::end(all_stars), z);
      }

      else
      {
        if(y >= 0 && x >= 0 && z >= 0)
        {
          x += min_dis_to_sun - euclid;
          y += min_dis_to_sun - euclid;
          z += min_dis_to_sun - euclid;

          all_stars.insert(std::end(all_stars), x);
          all_stars.insert(std::end(all_stars), y);
          all_stars.insert(std::end(all_stars), z);
        }

        else if(y < 0 && x >= 0 && z >= 0)
        {
          x += min_dis_to_sun - euclid;
          y -= min_dis_to_sun - euclid;
          z += min_dis_to_sun - euclid;

          all_stars.insert(std::end(all_stars), x);
          all_stars.insert(std::end(all_stars), y);
          all_stars.insert(std::end(all_stars), z);
        }

        else if(y >= 0 && x < 0 && z >= 0)
        {
          x -= min_dis_to_sun - euclid;
          y += min_dis_to_sun - euclid;
          z += min_dis_to_sun - euclid;

          all_stars.insert(std::end(all_stars), x);
          all_stars.insert(std::end(all_stars), y);
          all_stars.insert(std::end(all_stars), z);
        }

        else if(y >= 0 && x >= 0 && z < 0)
        {
          x += min_dis_to_sun - euclid;
          y += min_dis_to_sun - euclid;
          z -= min_dis_to_sun - euclid;

          all_stars.insert(std::end(all_stars), x);
          all_stars.insert(std::end(all_stars), y);
          all_stars.insert(std::end(all_stars), z);
        }

        else if(y < 0 && x < 0 && z >= 0)
        {
          x -= min_dis_to_sun - euclid;
          y -= min_dis_to_sun - euclid;
          z += min_dis_to_sun - euclid;

          all_stars.insert(std::end(all_stars), x);
          all_stars.insert(std::end(all_stars), y);
          all_stars.insert(std::end(all_stars), z);
        }

        else if(y < 0 && x >= 0 && z < 0)
        {
          x += min_dis_to_sun - euclid;
          y -= min_dis_to_sun - euclid;
          z -= min_dis_to_sun - euclid;

          all_stars.insert(std::end(all_stars), x);
          all_stars.insert(std::end(all_stars), y);
          all_stars.insert(std::end(all_stars), z);
        }

        else if(y >= 0 && x < 0 && z < 0)
        {
          x -= min_dis_to_sun - euclid;
          y += min_dis_to_sun - euclid;
          z -= min_dis_to_sun - euclid;

          all_stars.insert(std::end(all_stars), x);
          all_stars.insert(std::end(all_stars), y);
          all_stars.insert(std::end(all_stars), z);
        }

        else
        {
        x -= min_dis_to_sun - euclid;
        y -= min_dis_to_sun - euclid;
        z -= min_dis_to_sun - euclid;  

        all_stars.insert(std::end(all_stars), x);
        all_stars.insert(std::end(all_stars), y);
        all_stars.insert(std::end(all_stars), z);
        }
      }    
    }

    //3 Colour Values
    else
    {
      all_stars.insert(std::end(all_stars),1);    
    }
  }
}


void ApplicationSolar::SortParticles()
{
  std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void ApplicationSolar::fillParticles()
{
  float lfTime = 0.0f;
  int ParticlesCount = 0;
  float currTime = 0.0f;
  float delta = 0.0f;

  for(int i=0; i<MaxParticles; i++)
  {
    currTime = glfwGetTime();
    delta = currTime - lfTime;
    particle& p = ParticlesContainer[i]; // shortcut

    if(p.life > 0.0f)
    {

      // Decrease life
      p.life -= delta;
      if (p.life > 0.0f)
      {
        // Simulate simple physics : gravity only, no collisions
        p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
        p.pos += p.speed * (float)delta;
        // CameraPosition unkown
        p.cameradistance = glm::length( p.pos - CameraPosition ) * glm::length( p.pos - CameraPosition );
        //ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

        // Fill the GPU buffer
        // doesnt know these variables, cause they fail in the initialize
        g_particule_position_size_data[4*ParticlesCount+0] = p.pos.x;
        g_particule_position_size_data[4*ParticlesCount+1] = p.pos.y;
        g_particule_position_size_data[4*ParticlesCount+2] = p.pos.z;
                         
        g_particule_position_size_data[4*ParticlesCount+3] = p.size;
                         
        g_particule_color_data[4*ParticlesCount+0] = p.r;
        g_particule_color_data[4*ParticlesCount+1] = p.g;
        g_particule_color_data[4*ParticlesCount+2] = p.b;
        g_particule_color_data[4*ParticlesCount+3] = p.a;
      }

      else
      {
        // Particles that just died will be put at the end of the buffer in SortParticles();
        p.cameradistance = -1.0f;
      }

      lfTime = glfwGetTime();
      ParticlesCount++;
    }
  }

  SortParticles();
}

int ApplicationSolar::FindUnusedParticle()
{
    for(int i=LastUsedParticle; i<MaxParticles; i++){
        if (ParticlesContainer[i].life < 0){
            LastUsedParticle = i;
            return i;
        }
    }

    for(int i=0; i<LastUsedParticle; i++){
        if (ParticlesContainer[i].life < 0){
            LastUsedParticle = i;
            return i;
        }
    }

    return 0; // All particles are taken, override the first one
}


//fills the stars vector wiht suitable values
void ApplicationSolar::fillOrbits()
{
  for(int i = 0; i <= 360; i++)
  {
    all_orbits.push_back((float)cos((i*M_PI)/180));
    all_orbits.push_back(0.0f);
    all_orbits.push_back((float)sin((i*M_PI)/180));
  }
}


//initializes our planets and satellites
void ApplicationSolar::fillPlanets()
{
  texture_object t0, t1,t2,t3,t4,t5,t6,t7,t8,t9,t10;
  texture_object nt0, nt1,nt2,nt3,nt4,nt5,nt6,nt7,nt8,nt9,nt10;

  planet_texture.insert(std::end(planet_texture), {t0, t1,t2,t3,t4,t5,t6,t7,t8,t9,t10});
  normal_texture.insert(std::end(normal_texture), {nt0, nt1,nt2,nt3,nt4,nt5,nt6,nt7,nt8,nt9,nt10});

  planet sonne(10.0f, 0.0f, 0.0f); //Sun has to be initialized first for stuff to work
  planet merkur(0.6f, 0.48f, 15.0f);
  planet venus(2.5f, 0.35f, 22.0f);
  planet erde(2.9f, 0.3f, 28.0f);
  satellite mond(erde, 0.2f, 0.5f, 3.2f);
  planet mars(0.6f, 0.24f, 35.0f);
  planet jupiter(4.0f, 0.13f, 45.0f);
  planet saturn(3.0f, 0.1f, 55.0f);
  planet uranus(3.2f, 0.07f, 65.0f);
  planet neptun(2.9f, 0.05f, 75.0f);
  //planet pluto(0.5f, 3.6f, 62.0f);

  all_planets.insert(std::end(all_planets), {sonne, merkur, venus, erde, mars,
    jupiter,saturn, uranus, neptun});

  all_satellites.insert(std::end(all_satellites), {mond});
}


//calculates the matrices of the satellites according to the fathers matrices and draws the satellites
void ApplicationSolar::upload_planet_transforms(satellite const& p) const
{
  glUseProgram(m_shaders.at("planet").handle);

  //calcs the matrices of the father planets
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()*p.m_father.m_rot_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -p.m_father.m_dis_to_origin});

  //Calcs local matrices of the moons with those
  //of the father planets
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime()*p.m_rot_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -p.m_dis_to_father});
  model_matrix = glm::scale(model_matrix, glm::fvec3{p.m_size, p.m_size, p.m_size});
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));


  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, planet_texture[10].handle);
  int color_sampler_location = glGetUniformLocation(m_shaders.at("planet").handle, "ColorTex");
  glUniform1i(color_sampler_location, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, normal_texture[10].handle);
  glUniform1i(glGetUniformLocation(m_shaders.at("planet").handle, "NormalTex"), 1);

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                     1, GL_FALSE, glm::value_ptr(normal_matrix));

  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);// GL draw array für Sterne
}


void ApplicationSolar::upload_planet_transforms(planet const& p, int k) const
{
  //calcs the matrices of the planets
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()*p.m_rot_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -p.m_dis_to_origin});
  model_matrix = glm::scale(model_matrix, glm::fvec3{p.m_size, p.m_size, p.m_size});

  if (p.m_dis_to_origin == 0.0)
  {
    glUseProgram(m_shaders.at("sun").handle);
    glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ModelMatrix"),
                     1, GL_FALSE, glm::value_ptr(model_matrix));


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planet_texture[k+1].handle);
    int color_sampler_location = glGetUniformLocation(m_shaders.at("planet").handle, "ColorTex");
    glUniform1i(color_sampler_location, 0);

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }

  else
  {
    glUseProgram(m_shaders.at("planet").handle);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                    1, GL_FALSE, glm::value_ptr(model_matrix));

    //extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                    1, GL_FALSE, glm::value_ptr(normal_matrix));
    

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planet_texture[k+1].handle);
    int color_sampler_location = glGetUniformLocation(m_shaders.at("planet").handle, "ColorTex");
    glUniform1i(color_sampler_location, 0);

     // Bind our normal texture in Texture Unit 1
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal_texture[k+1].handle);
    glUniform1i(glGetUniformLocation(m_shaders.at("planet").handle, "NormalTex"), 1);



    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }
}

void ApplicationSolar::upload_starsphere() const
{
  //calcs the matrices of the planets    nur texturkoordinaton / Projection 
  glUseProgram(m_shaders.at("skysphere").handle);

  //extra matrix for normal transformation to keep them orthogonal to surface

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, planet_texture[0].handle);
  int color_sampler_location = glGetUniformLocation(m_shaders.at("skysphere").handle, "ColorTex");
  glUniform1i(color_sampler_location, 0);


  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
}


void ApplicationSolar::upload_stars() const
{
  glUseProgram(m_shaders.at("star").handle);
  glBindVertexArray(star_object.vertex_AO);
  glPointSize(1);
  glDrawArrays(star_object.draw_mode, 0, (int)all_stars.size());
}

void ApplicationSolar::upload_particles() const
{
  glUseProgram(m_shaders.at("particle").handle);

  double lastTime = glfwGetTime();

  //update buffers
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

  // 1 attribute: vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // 2 attribute: positions of particles' centers
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

  // 3 attribute: particles´ colour
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0);

  // tell OpenGl, which is for what
  glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
  glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
  glVertexAttribDivisor(2, 1); // color : one per quad -> 1

  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);
}

void ApplicationSolar::upload_orbits(planet const& p) const
{
  float dist = p.m_dis_to_origin;
  //stretches the Circles to the distance of each planets.
  glm::fmat4 model_matrix = glm::scale(glm::fmat4{}, glm::fvec3{dist, 0, dist});

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

  glBindVertexArray(orbit_object.vertex_AO);
  glDrawArrays(orbit_object.draw_mode, 0, orbit_object.num_elements);
}


void ApplicationSolar::upload_orbits(satellite const& p) const
{
  float dist = p.m_dis_to_father;
  //stretches the Circles to the distance of each planets.
  glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()*p.m_father.m_rot_speed), glm::fvec3{0.0f, 1.0f, 0.0f});
  model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -p.m_father.m_dis_to_origin});

  //stretches it around the father planet
  model_matrix = glm::scale(model_matrix, glm::fvec3{dist, 0, dist});

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

  glBindVertexArray(orbit_object.vertex_AO);
  glDrawArrays(orbit_object.draw_mode, 0, orbit_object.num_elements);
}

void ApplicationSolar::upload_quad() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUseProgram(m_shaders.at("simple_quad").handle);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, frame_buffer_texture.handle);

  int color_sampler_location = glGetUniformLocation(m_shaders.at("simple_quad").handle, "ColorTex");
  glUniform1i(color_sampler_location, 0);

  glBindVertexArray(screen_quad_object.vertex_AO);

  glDrawArrays(screen_quad_object.draw_mode, 0, screen_quad_object.num_elements); 
}

void ApplicationSolar::render() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object.handle);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glDisable(GL_DEPTH_TEST);
  upload_starsphere();
  glEnable(GL_DEPTH_TEST);


  for (unsigned int i = 0; i < all_planets.size(); i++)
  {
    upload_planet_transforms(all_planets[i], i);
    upload_orbits(all_planets[i]);
  }

  for (unsigned int i = 0; i < all_satellites.size(); i++){
    upload_planet_transforms(all_satellites[i]);
    upload_orbits(all_satellites[i]);
  }

  upload_stars();
  upload_particles();
  upload_quad();
}


void ApplicationSolar::updateView()
{
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);

  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
  glUniform1f(m_shaders.at("planet").u_locs.at("ShadingMethod"), m_shading_method);


  glUseProgram(m_shaders.at("skysphere").handle);
  glUniformMatrix4fv(m_shaders.at("skysphere").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));


  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("particle").handle);
  glUniformMatrix4fv(m_shaders.at("particle").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("simple_quad").handle);
  glUniform1f(m_shaders.at("simple_quad").u_locs.at("H_Mirrored"), m_h_mirrored);
  glUniform1f(m_shaders.at("simple_quad").u_locs.at("V_Mirrored"), m_v_mirrored);
  glUniform1f(m_shaders.at("simple_quad").u_locs.at("GrayScale"), m_grayscale);
  glUniform1f(m_shaders.at("simple_quad").u_locs.at("Blur"), m_blur);
  glUniform1f(m_shaders.at("simple_quad").u_locs.at("Sobel"), m_sobel);
}


void ApplicationSolar::updateProjection()
{
  // upload matrix to gpu
  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("skysphere").handle);
  glUniformMatrix4fv(m_shaders.at("skysphere").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
 
  glUseProgram(m_shaders.at("particle").handle);
  glUniformMatrix4fv(m_shaders.at("particle").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
  
  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}


// update uniform locations
void ApplicationSolar::uploadUniforms()
{
  updateUniformLocations();
  
  // bind new shader
  glUseProgram(m_shaders.at("sun").handle);
  glUseProgram(m_shaders.at("planet").handle);
  glUseProgram(m_shaders.at("orbit").handle);
  glUseProgram(m_shaders.at("simple_quad").handle);
  
  updateView();
  updateProjection();
}


// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods)
{
  //Forwards
  if (key == GLFW_KEY_W)
  {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.5f});
    updateView();
  }

  //Backwards
  else if (key == GLFW_KEY_S)
  {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.5f});
    updateView();
  }

  //Left
  else if (key == GLFW_KEY_A)
  {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.5f, 0.0f, 0.0f});
    updateView();
  }

  //Right
  else if (key == GLFW_KEY_D)
  {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.5f, 0.0f, 0.0f});
    updateView();
  }

  //Up
  else if (key == GLFW_KEY_SPACE)
  {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.5f, 0.0f});
    updateView();
  }

  //Down
  else if (key == GLFW_KEY_X)
  {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, -0.5f, 0.0f});
    updateView();
  }

  //Bill-Phong Shading
  else if (key == GLFW_KEY_1)
  {
    m_shading_method = 0.0f;//glUniform1f(m_shaders.at("planet").u_locs.at("ShadingMethod"), GLfloat(0.0f));
    updateView();
  }

  //Cel shading
  else if (key == GLFW_KEY_2)
  {
    m_shading_method = 1.0f;//glUniform1f(m_shaders.at("planet").u_locs.at("ShadingMethod"), GLfloat(1.0f));
    updateView();
  }

  // grayscale
  else if (key == GLFW_KEY_6 && action == GLFW_PRESS)
  {
    m_sobel == false ? m_sobel = true : m_sobel = false;
    updateView();
  }

  // grayscale
  else if (key == GLFW_KEY_7 && action == GLFW_PRESS)
  {
    m_grayscale == false ? m_grayscale = true : m_grayscale = false;
    updateView();
  }

  // mirror at x-axis
  else if (key == GLFW_KEY_8 && action == GLFW_PRESS)
  {
    m_h_mirrored == false ? m_h_mirrored = true : m_h_mirrored = false;
    updateView();
  }

  // mirror at y axis
  else if (key == GLFW_KEY_9 && action == GLFW_PRESS)
  {
    m_v_mirrored == false ? m_v_mirrored = true : m_v_mirrored = false;
    updateView();
  }

  // bluuuuuuuuuuuuuur
  else if (key == GLFW_KEY_0 && action == GLFW_PRESS)
  {
    m_blur == false ? m_blur = true : m_blur = false;
    updateView();
  }

}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y)
{
  m_view_transform = glm::rotate(m_view_transform, (float)pos_y/100, glm::fvec3(-1.0f, 0.0f, 0.0f));
  m_view_transform = glm::rotate(m_view_transform, (float)pos_x/100, glm::fvec3(0.0f, -1.0f, 0.0f));

  updateView();
}


// load shader programs
void ApplicationSolar::initializeShaderPrograms()
{
 // store shader program objects in container
  m_shaders.emplace("sun", shader_program{m_resource_path + "shaders/sun.vert",
                                           m_resource_path + "shaders/sun.frag"});
  // request uniform locations for shader program
  m_shaders.at("sun").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sun").u_locs["ViewMatrix"] = -1;
  m_shaders.at("sun").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("sun").u_locs["ColorTex"] = -1;

  // request uniform locations for shader program
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/planet.vert",
                                           m_resource_path + "shaders/planet.frag"});
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["ShadingMethod"] = -1;
  m_shaders.at("planet").u_locs["ColorTex"] = -1;
  m_shaders.at("planet").u_locs["NormalTex"] = -1;

  // request uniform locations for shader program
  m_shaders.emplace("skysphere", shader_program{m_resource_path + "shaders/skysphere.vert",
                                           m_resource_path + "shaders/skysphere.frag"});
  m_shaders.at("skysphere").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("skysphere").u_locs["ViewMatrix"] = -1;
  m_shaders.at("skysphere").u_locs["ColorTex"] = -1;
  

  m_shaders.emplace("star", 
    shader_program{m_resource_path + "shaders/star.vert",
  m_resource_path + "shaders/star.frag"});

  //wie in den shader dateien: keine model/normal notwendig
  m_shaders.at("star").u_locs["ViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;

  m_shaders.at("particle").u_locs["ViewMatrix"] = -1;
  m_shaders.at("particle").u_locs["ProjectionMatrix"] = -1;

  // request uniform locations for orbit.shader
  m_shaders.emplace("orbit", shader_program{m_resource_path + "shaders/orbit.vert",
                                          m_resource_path + "shaders/orbit.frag"});
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;

  //request uniform locations for simple_quad.shader
  m_shaders.emplace("simple_quad", shader_program{m_resource_path + "shaders/simple_quad.vert",
                                           m_resource_path + "shaders/simple_quad.frag"});
  m_shaders.at("simple_quad").u_locs["ColorTex"] = -1;
  m_shaders.at("simple_quad").u_locs["GrayScale"] = -1;
  m_shaders.at("simple_quad").u_locs["V_Mirrored"] = -1;
  m_shaders.at("simple_quad").u_locs["H_Mirrored"] = -1;
  m_shaders.at("simple_quad").u_locs["Blur"] = -1;
  m_shaders.at("simple_quad").u_locs["Sobel"] = -1;
}


// load models
void ApplicationSolar::initializeGeometry()
{
  //+++++++++++++++++++++++PlANETS++++++++++++++++++++

  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD | model::TANGENT);

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);


  glEnableVertexAttribArray(2);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);


  glEnableVertexAttribArray(3);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(3, model::TANGENT.components, model::TANGENT.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TANGENT]);

   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());

}


// load stars
void ApplicationSolar::initializeStars()
{
//+++++++++++++++++STARS+++++++++++++++++++++++++++++++++

  model star_model = {all_stars, model::POSITION | model::NORMAL};

  // generate vertex array object
  glGenVertexArrays(1, &star_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(star_object.vertex_AO);
  // generate generic buffer
  glGenBuffers(1, &star_object.vertex_BO);// DAS HIER SELBEER MACHEN
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  // configure currently bound array buffer -> We can use size of container, because we don't have spheres
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * all_stars.size(), all_stars.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float),(void*)0);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));


   // generate generic buffer
  glGenBuffers(1, &star_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, star_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * sizeof(float), all_stars.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  star_object.draw_mode = GL_POINTS;
  // transfer number of indices to model object 
  star_object.num_elements = GLsizei(all_stars.size()/6);
}


// load Orbits
void ApplicationSolar::initializeOrbits()
{
//+++++++++++++++++ORBITS+++++++++++++++++++++++++++++++++

  model orbit_model = model{all_orbits, model::POSITION, {1}};

  // generate vertex array object
  glGenVertexArrays(1, &orbit_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(orbit_object.vertex_AO);
  // generate generic buffer
  glGenBuffers(1, &orbit_object.vertex_BO);// DAS HIER SELBEER MACHEN
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  // configure currently bound array buffer -> We can use size of container, because we don't have spheres
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * all_orbits.size(), all_orbits.data(), GL_STATIC_DRAW);

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, orbit_model.vertex_bytes, orbit_model.offsets[model::POSITION]);

  // store type of primitive to draw
  orbit_object.draw_mode = GL_LINE_LOOP;
  //transfer number of indices to model object 
  orbit_object.num_elements = GLsizei(all_orbits.size()/3);
}

void ApplicationSolar::initializeTextures()
{ 
  for (unsigned int i = 0; i < 10; i++)
  {
    std::string path = m_resource_path + "textures/" + std::to_string(i) + ".png";
    pixel_data pix_dat = texture_loader::file(path); 

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &planet_texture[i].handle);
    glBindTexture(GL_TEXTURE_2D, planet_texture[i].handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, pix_dat.channels, pix_dat.width, pix_dat.height, 0, pix_dat.channels, pix_dat.channel_type, pix_dat.ptr());

    std::string path_normals = m_resource_path + "normal_textures/" + std::to_string(i) + ".png";
    pixel_data pix_dat_normal = texture_loader::file(path_normals); 

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &normal_texture[i].handle);
    glBindTexture(GL_TEXTURE_2D, normal_texture[i].handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, pix_dat_normal.channels, pix_dat_normal.width, pix_dat_normal.height, 0, pix_dat_normal.channels, pix_dat_normal.channel_type, pix_dat_normal.ptr());
  }
}

void ApplicationSolar::initializeFramebuffer()
{
  glGenRenderbuffers(1, &render_buffer_object.handle);
  glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_object.handle);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, GLsizei(1920u), GLsizei(1080u));

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &frame_buffer_texture.handle);
  glBindTexture(GL_TEXTURE_2D, frame_buffer_texture.handle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GLsizei(1920u), GLsizei(1080u), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glGenFramebuffers(1, &frame_buffer_object.handle);
  glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object.handle); 
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frame_buffer_texture.handle, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer_object.handle);

  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cout << "FRAMEBUFFER just didn`t" << std::endl;
    }
}

// load screen quad
void ApplicationSolar::initializeScreenQuad()
{
  all_quads.insert(std::end(all_quads),{
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f});

  model screen_quad_model = {all_quads, model::POSITION | model::TEXCOORD, {1}};

  glGenVertexArrays(1, &screen_quad_object.vertex_AO);
  glBindVertexArray(screen_quad_object.vertex_AO);

  glGenBuffers(1, &screen_quad_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, screen_quad_object.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * screen_quad_model.data.size(), screen_quad_model.data.data(), GL_STATIC_DRAW);  

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, screen_quad_model.vertex_bytes, screen_quad_model.offsets[model::POSITION]);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, screen_quad_model.vertex_bytes, screen_quad_model.offsets[model::TEXCOORD]);

  screen_quad_object.draw_mode = GL_TRIANGLE_STRIP;
  screen_quad_object.num_elements = GLsizei(screen_quad_model.data.size()/5);

  glBindVertexArray(0); 
}

void ApplicationSolar::initializeParticles(){
  
  static const GLfloat g_vertex_buffer_data[] = 
  {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f,
  };

  // this doesnt work somehow. declaration failed.
  GLuint billboard_vertex_buffer;
  glGenBuffers(1, &billboard_vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  // The VBO containing the positions and sizes of the particles
  GLuint particles_position_buffer;
  glGenBuffers(1, &particles_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

 
  // The VBO containing the colors of the particles
  GLuint particles_color_buffer;
  glGenBuffers(1, &particles_color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}


ApplicationSolar::~ApplicationSolar()
{
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);

  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);

  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteBuffers(1, &orbit_object.element_BO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);

  //glDeleteBuffers(1, &particles_color_buffer);
  //glDeleteBuffers(1, &particles_position_buffer);
  //glDeleteBuffers(1, &billboard_vertex_buffer);

  //glDeleteFramebuffers(1, &frame_buffer_object);
}

// exe entry point
int main(int argc, char* argv[])
{
  Launcher::run<ApplicationSolar>(argc, argv);
}
