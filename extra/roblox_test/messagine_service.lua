local MessagingService = game:GetService("MessagingService")

MessagingService:SubscribeAsync(
	"TestTopic",
	function(message)
		print(message.Data)
	end
)
