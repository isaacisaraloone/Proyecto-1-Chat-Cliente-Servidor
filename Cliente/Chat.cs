using System;
using System.Text.Json;
using System.Threading;

namespace Cliente
{
	public class Chat
	{
		private ManejarConexion _conexion;
		private AnalizarComando _analizador;
		private string _usuario = "";
		private bool _ejecutando;

		public Chat()
		{
			_conexion = new ManejarConexion();
			_analizador = new AnalizarComando();
			_ejecutando = false;
		}
		public void Iniciar(string ip, int port)
		{
			try
			{
				_conexion.Conectar(ip, port);
				Console.WriteLine("Conectado al servidor.");
				if (!Autenticar())
				{
					return;
				}
				_ejecutando = true;
				Thread recibirHilo = new Thread(EscucharMensajes);
				recibirHilo.Start();
				_analizador.AyudaVisual();
				while (_ejecutando)
				{
					string? input = Console.ReadLine();
					if (string.IsNullOrEmpty(input))
					{
						continue;
					}
					var json = _analizador.ProcesarEntradaUsuario(input);
					foreach (var mensaje in json)
					{
						_conexion.EnviarMensaje(mensaje);
					}
					if (input.Trim().ToLower() == "/quit")
					{
						_ejecutando = false;
					}
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error de aplicacion: {ex.Message}");
			}
			finally
			{
				_ejecutando = false;
				_conexion.Dispose();
			}
		}
		private bool Autenticar()
		{
			Console.Write("Ingresa tu nombre de usuario (maximo 8 caracteres): ");
			string? entradaUsuario = Console.ReadLine()?.Trim();
			if (string.IsNullOrEmpty(entradaUsuario) || entradaUsuario.Length > 8)
			{
				Console.WriteLine("Nombre de usuario no valido.");
				return false;
			}
			_usuario = entradaUsuario;
			string identificarMsj = _analizador.GenerarIdentificacion(_usuario);
			_conexion.EnviarMensaje(identificarMsj);
			string? responseLine = _conexion.LeerMensaje();
			if (responseLine == null)
			{
				Console.WriteLine("El servidor cerro la conexion.");
				return false;
			}
			try
			{
				var doc = JsonDocument.Parse(responseLine);
				var raiz = doc.RootElement;
				if (raiz.TryGetProperty("type", out var tipo) && tipo.GetString() == "RESPONSE" &&
					raiz.TryGetProperty("result", out var resultado) && resultado.GetString() == "SUCCESS")
				{
					Console.WriteLine($"Identificacion exitosa como {_usuario}.");
					return true;
				}
				else
				{
					Console.WriteLine($"Error al identificar");
					return false;
				}
			}
			catch
			{
				Console.WriteLine("Respuesta inesperada.");
				return false;
			}
		}
		private void EscucharMensajes()
		{
			try
			{
				while (_ejecutando)
				{
					string? line = _conexion.LeerMensaje();
					if (line == null)
					{
						if (_ejecutando)
						{
							Console.WriteLine("Desconectado del servidor.");
							_ejecutando = false;
							break;
						}
					}

					if (!string.IsNullOrWhiteSpace(line))
					{
						_analizador.ProcesarMensajeServidor(line);
					}
				}
			}
			catch (Exception)
			{
				if (_ejecutando)
				{
					Console.WriteLine("Conexion cerrada por el servidor.");
				}
			}
		}
	}
}